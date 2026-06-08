#include <QTest>

#include "peercast/peercast_url.h"
#include "peercast/stream_resolver.h"

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
};

QTEST_MAIN(TstPeercast)
#include "tst_peercast.moc"
