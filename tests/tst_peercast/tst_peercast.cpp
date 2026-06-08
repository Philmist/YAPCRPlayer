#include <QTest>

#include "peercast/peercast_url.h"
#include "peercast/stream_resolver.h"
#include "peercast/channel_info.h"

// StreamResolver の analyse() はプライベートメソッドなのでテスト用に公開するために
// ここでは同等の実装を直接 QRegularExpression で検証する。
#include <QRegularExpression>

class TstPeercast : public QObject {
    Q_OBJECT

private slots:
    // ---- PeerCastUrl::parse ----

    void parse_valid_basic() {
        const auto r = yapcr::peercast::PeerCastUrl::parse(
            QStringLiteral("http://127.0.0.1:7144/pls/0123456789ABCDEF0123456789ABCDEF"));
        QVERIFY(r.valid);
        QCOMPARE(r.host, QStringLiteral("127.0.0.1"));
        QCOMPARE(r.port, QStringLiteral("7144"));
        QCOMPARE(r.id,   QStringLiteral("0123456789ABCDEF0123456789ABCDEF"));
    }

    void parse_valid_with_tip_param() {
        const auto r = yapcr::peercast::PeerCastUrl::parse(
            QStringLiteral("http://127.0.0.1:7144/pls/AAAABBBBCCCCDDDD1111222233334444?tip=1.2.3.4:7144"));
        QVERIFY(r.valid);
        QCOMPARE(r.id, QStringLiteral("AAAABBBBCCCCDDDD1111222233334444"));
    }

    void parse_valid_host_with_name() {
        const auto r = yapcr::peercast::PeerCastUrl::parse(
            QStringLiteral("http://localhost:7144/pls/ABCDEF1234567890ABCDEF1234567890"));
        QVERIFY(r.valid);
        QCOMPARE(r.host, QStringLiteral("localhost"));
    }

    void parse_invalid_local_file() {
        const auto r = yapcr::peercast::PeerCastUrl::parse(
            QStringLiteral("C:/Users/user/video.mkv"));
        QVERIFY(!r.valid);
    }

    void parse_invalid_id_too_short() {
        // ID が 31 文字（1 文字不足）
        const auto r = yapcr::peercast::PeerCastUrl::parse(
            QStringLiteral("http://127.0.0.1:7144/pls/AAAABBBBCCCCDDDD111122223333444"));
        QVERIFY(!r.valid);
    }

    void parse_invalid_id_too_long() {
        // ID が 33 文字（1 文字過剰）—— URL 内の /stream/ URL 等に誤マッチしないこと
        // 注: QRegularExpression は {32} の後は境界でなくてもよい（先頭マッチ ^）。
        // PCRPlayer 原典も同じ動作: regex_search はパターンが一致する先頭を返す。
        // /pls/ の後に 33 文字ある URL は先頭 32 文字でマッチするので valid になる。
        // この仕様は原典と同一なので期待値を valid=true にしている。
        const auto r = yapcr::peercast::PeerCastUrl::parse(
            QStringLiteral("http://127.0.0.1:7144/pls/AAAABBBBCCCCDDDD1111222233334444X"));
        QVERIFY(r.valid);  // 先頭 32 文字でマッチ
        QCOMPARE(r.id, QStringLiteral("AAAABBBBCCCCDDDD1111222233334444"));
    }

    void parse_invalid_not_pls() {
        const auto r = yapcr::peercast::PeerCastUrl::parse(
            QStringLiteral("http://127.0.0.1:7144/stream/AAAABBBBCCCCDDDD1111222233334444"));
        QVERIFY(!r.valid);
    }

    void parse_invalid_https() {
        // https:// は PCRPlayer 原典もサポートしない（http:// のみ）
        const auto r = yapcr::peercast::PeerCastUrl::parse(
            QStringLiteral("https://127.0.0.1:7144/pls/AAAABBBBCCCCDDDD1111222233334444"));
        QVERIFY(!r.valid);
    }

    // ---- StreamResolver::analyse に相当する正規表現のテスト ----
    // analyse() はプライベートなので、同等の正規表現を直接テストする。

    void analyse_http_no_ext() {
        static const QRegularExpression rx(
            QStringLiteral(R"([0-9a-z]+://[^/]+/stream/[0-9a-zA-Z]{32}(\.[_0-9a-zA-Z]+)?)"));
        const QString playlist = QStringLiteral(
            "http://127.0.0.1:7144/stream/AAAABBBBCCCCDDDD1111222233334444");
        const auto m = rx.match(playlist);
        QVERIFY(m.hasMatch());
        QCOMPARE(m.captured(0),
                 QStringLiteral("http://127.0.0.1:7144/stream/AAAABBBBCCCCDDDD1111222233334444"));
    }

    void analyse_http_with_ts_ext() {
        static const QRegularExpression rx(
            QStringLiteral(R"([0-9a-z]+://[^/]+/stream/[0-9a-zA-Z]{32}(\.[_0-9a-zA-Z]+)?)"));
        const QString playlist = QStringLiteral(
            "http://127.0.0.1:7144/stream/AAAABBBBCCCCDDDD1111222233334444.ts");
        const auto m = rx.match(playlist);
        QVERIFY(m.hasMatch());
        QVERIFY(m.captured(0).endsWith(QStringLiteral(".ts")));
    }

    void analyse_http_with_flv_ext() {
        static const QRegularExpression rx(
            QStringLiteral(R"([0-9a-z]+://[^/]+/stream/[0-9a-zA-Z]{32}(\.[_0-9a-zA-Z]+)?)"));
        const QString playlist = QStringLiteral(
            "http://localhost:7144/stream/00112233445566778899AABBCCDDEEFF.flv");
        const auto m = rx.match(playlist);
        QVERIFY(m.hasMatch());
        QVERIFY(m.captured(0).endsWith(QStringLiteral(".flv")));
    }

    void analyse_embedded_in_m3u() {
        // .pls や .m3u 形式のプレイリスト本文の中から stream URL を抽出する
        static const QRegularExpression rx(
            QStringLiteral(R"([0-9a-z]+://[^/]+/stream/[0-9a-zA-Z]{32}(\.[_0-9a-zA-Z]+)?)"));
        const QString playlist = QStringLiteral(
            "[playlist]\n"
            "NumberOfEntries=1\n"
            "File1=http://127.0.0.1:7144/stream/AAAABBBBCCCCDDDD1111222233334444.ts\n"
            "Title1=TestChannel\n"
            "Version=2\n");
        const auto m = rx.match(playlist);
        QVERIFY(m.hasMatch());
        QCOMPARE(m.captured(0),
                 QStringLiteral("http://127.0.0.1:7144/stream/AAAABBBBCCCCDDDD1111222233334444.ts"));
    }

    void analyse_no_match() {
        static const QRegularExpression rx(
            QStringLiteral(R"([0-9a-z]+://[^/]+/stream/[0-9a-zA-Z]{32}(\.[_0-9a-zA-Z]+)?)"));
        const QString playlist = QStringLiteral("no stream url here");
        const auto m = rx.match(playlist);
        QVERIFY(!m.hasMatch());
    }

    // ---- parseViewXml ----

    // フィクスチャ: PeerCastStation の viewxml 応答を模したテスト用 XML
    // 全フィールドが XML 属性（PCRPlayer XmlParser.h の <xmlattr>. で確認済み）
    static QByteArray makeViewXml(const QString& targetId) {
        return QStringLiteral(
            "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
            "<peercast>"
            "  <channels_relayed>"
            "    <channel id=\"AAAA1111BBBB2222CCCC3333DDDD4444\" name=\"Other Channel\""
            "             bitrate=\"128\" type=\"MP3\" genre=\"Other\" desc=\"other desc\""
            "             url=\"http://example.com\" uptime=\"01:00:00\""
            "             comment=\"other comment\" skips=\"0\" age=\"0\" bcflags=\"0\">"
            "      <relay listeners=\"0\" relays=\"0\" hosts=\"1\""
            "             status=\"RECEIVING\" firewalled=\"0\"/>"
            "      <track title=\"Other Title\" artist=\"Other Artist\""
            "             album=\"\" genre=\"\" contact=\"\"/>"
            "    </channel>"
            "    <channel id=\"%1\" name=\"Test Channel\""
            "             bitrate=\"256\" type=\"FLV\" genre=\"アニメ\" desc=\"テスト放送\""
            "             url=\"http://test.example.com\" uptime=\"00:30:00\""
            "             comment=\"コメントです\" skips=\"0\" age=\"0\" bcflags=\"0\">"
            "      <relay listeners=\"3\" relays=\"1\" hosts=\"2\""
            "             status=\"RECEIVING\" firewalled=\"0\"/>"
            "      <track title=\"テストタイトル\" artist=\"テストアーティスト\""
            "             album=\"テストアルバム\" genre=\"\" contact=\"http://contact.example.com\"/>"
            "    </channel>"
            "  </channels_relayed>"
            "</peercast>")
            .arg(targetId)
            .toUtf8();
    }

    void parseViewXml_found_matching_channel() {
        const QString id = QStringLiteral("1234567890ABCDEF1234567890ABCDEF");
        const auto info = yapcr::peercast::parseViewXml(makeViewXml(id), id);

        QVERIFY(info.valid);
        QCOMPARE(info.id,      id);
        QCOMPARE(info.name,    QStringLiteral("Test Channel"));
        QCOMPARE(info.bitrate, QStringLiteral("256"));
        QCOMPARE(info.type,    QStringLiteral("FLV"));
        QCOMPARE(info.genre,   QStringLiteral("アニメ"));
        QCOMPARE(info.desc,    QStringLiteral("テスト放送"));
        QCOMPARE(info.comment, QStringLiteral("コメントです"));
    }

    void parseViewXml_relay_attributes() {
        const QString id = QStringLiteral("1234567890ABCDEF1234567890ABCDEF");
        const auto info = yapcr::peercast::parseViewXml(makeViewXml(id), id);

        QVERIFY(info.valid);
        QCOMPARE(info.listeners, 3);
        QCOMPARE(info.relays,    1);
        QCOMPARE(info.status,    QStringLiteral("RECEIVING"));
        QVERIFY(!info.firewalled);
    }

    void parseViewXml_track_attributes() {
        const QString id = QStringLiteral("1234567890ABCDEF1234567890ABCDEF");
        const auto info = yapcr::peercast::parseViewXml(makeViewXml(id), id);

        QVERIFY(info.valid);
        QCOMPARE(info.trackTitle,   QStringLiteral("テストタイトル"));
        QCOMPARE(info.trackArtist,  QStringLiteral("テストアーティスト"));
        QCOMPARE(info.trackAlbum,   QStringLiteral("テストアルバム"));
        QCOMPARE(info.trackContact, QStringLiteral("http://contact.example.com"));
    }

    void parseViewXml_id_case_insensitive() {
        // ID は大文字小文字無視で一致（PCRPlayer の _wcsicmp に相当）
        const QString id      = QStringLiteral("1234567890abcdef1234567890abcdef");
        const QString idUpper = QStringLiteral("1234567890ABCDEF1234567890ABCDEF");
        const auto info = yapcr::peercast::parseViewXml(makeViewXml(idUpper), id);
        QVERIFY(info.valid);
    }

    void parseViewXml_no_match_returns_invalid() {
        const auto info = yapcr::peercast::parseViewXml(
            makeViewXml(QStringLiteral("1234567890ABCDEF1234567890ABCDEF")),
            QStringLiteral("FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF"));
        QVERIFY(!info.valid);
    }

    void parseViewXml_selects_correct_channel_from_multiple() {
        // AAAA... ではなく target ID のチャンネルが返ること
        const QString id = QStringLiteral("1234567890ABCDEF1234567890ABCDEF");
        const auto info = yapcr::peercast::parseViewXml(makeViewXml(id), id);
        QVERIFY(info.valid);
        QCOMPARE(info.name, QStringLiteral("Test Channel"));  // AAAA... の "Other Channel" ではない
    }

    void parseViewXml_empty_xml_returns_invalid() {
        const auto info = yapcr::peercast::parseViewXml(
            QByteArray{}, QStringLiteral("1234567890ABCDEF1234567890ABCDEF"));
        QVERIFY(!info.valid);
    }

    // ---- formatTitle ----

    void formatTitle_with_type() {
        yapcr::peercast::ChannelInfo info;
        info.valid = true;
        info.name  = QStringLiteral("テストチャンネル");
        info.type  = QStringLiteral("FLV");
        QCOMPARE(yapcr::peercast::formatTitle(info),
                 QStringLiteral("テストチャンネル (FLV)"));
    }

    void formatTitle_without_type() {
        yapcr::peercast::ChannelInfo info;
        info.valid = true;
        info.name  = QStringLiteral("テストチャンネル");
        QCOMPARE(yapcr::peercast::formatTitle(info),
                 QStringLiteral("テストチャンネル"));
    }

    void formatTitle_invalid_returns_empty() {
        QVERIFY(yapcr::peercast::formatTitle({}).isEmpty());
    }

    // ---- formatStatus ----

    void formatStatus_full_fields() {
        yapcr::peercast::ChannelInfo info;
        info.valid        = true;
        info.name         = QStringLiteral("チャンネル");
        info.type         = QStringLiteral("FLV");
        info.genre        = QStringLiteral("アニメ");
        info.desc         = QStringLiteral("説明");
        info.comment      = QStringLiteral("コメント");
        info.trackArtist  = QStringLiteral("アーティスト");
        info.trackTitle   = QStringLiteral("タイトル");
        info.listeners    = 5;
        info.relays       = 2;
        const QString status = yapcr::peercast::formatStatus(info);
        // 各フィールドが含まれているか確認
        QVERIFY(status.contains(QStringLiteral("チャンネル")));
        QVERIFY(status.contains(QStringLiteral("FLV")));
        QVERIFY(status.contains(QStringLiteral("アニメ")));
        QVERIFY(status.contains(QStringLiteral("説明")));
        QVERIFY(status.contains(QStringLiteral("コメント")));
        QVERIFY(status.contains(QStringLiteral("アーティスト")));
        QVERIFY(status.contains(QStringLiteral("タイトル")));
        QVERIFY(status.contains(QStringLiteral("5")));
        QVERIFY(status.contains(QStringLiteral("2")));
    }

    void formatStatus_empty_genre_desc_omits_bracket() {
        yapcr::peercast::ChannelInfo info;
        info.valid = true;
        info.name  = QStringLiteral("チャンネル");
        info.type  = QStringLiteral("MP3");
        // genre, desc が空
        const QString status = yapcr::peercast::formatStatus(info);
        QVERIFY(!status.contains(QStringLiteral("[")));
        QVERIFY(!status.contains(QStringLiteral("]")));
    }

    void formatStatus_no_listeners_omits_count() {
        yapcr::peercast::ChannelInfo info;
        info.valid     = true;
        info.name      = QStringLiteral("チャンネル");
        // listeners/relays = -1（未取得）
        const QString status = yapcr::peercast::formatStatus(info);
        // [N/M] 形式は現れない
        QVERIFY(!status.contains(QStringLiteral("/-1")));
        QVERIFY(!status.contains(QStringLiteral("-1/")));
    }

    void formatStatus_invalid_returns_empty() {
        QVERIFY(yapcr::peercast::formatStatus({}).isEmpty());
    }
};

QTEST_MAIN(TstPeercast)
#include "tst_peercast.moc"
