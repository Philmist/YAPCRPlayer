#include <QTest>

#include "bbs/board_url.h"
#include "bbs/charset.h"
#include "bbs/models.h"

// --------------------------------------------------------------------------
// ヘルパー: Board を手動生成して URL ビルダーを呼び出す
// --------------------------------------------------------------------------
static yapcr::bbs::Board makeJpnknBoard(
    const QString& scheme,
    const QString& base,
    const QString& board)
{
    yapcr::bbs::Board b;
    b.scheme = scheme;
    b.host   = base;   // jpnkn: base = host
    b.base   = base;
    b.board  = board;
    // number は空（jpnkn は board/number 分離なし）
    b.code   = yapcr::bbs::Charset::ShiftJis;
    return b;
}

static yapcr::bbs::Board makeShitarabaBoard(
    const QString& scheme,
    const QString& host,
    const QString& board,
    const QString& number)
{
    yapcr::bbs::Board b;
    b.scheme = scheme;
    b.host   = host;
    b.base   = host;   // したらば: base = host のみ
    b.board  = board;
    b.number = number;
    b.code   = yapcr::bbs::Charset::EucJp;
    return b;
}

// --------------------------------------------------------------------------

class TstBbs : public QObject {
    Q_OBJECT

private slots:

    // ======== detectBoardType ========

    void detectBoardType_shitaraba()
    {
        using namespace yapcr::bbs;
        QCOMPARE(detectBoardType(QStringLiteral("jbbs.shitaraba.net")),
                 BoardType::Shitaraba);
    }

    void detectBoardType_jpnkn()
    {
        using namespace yapcr::bbs;
        QCOMPARE(detectBoardType(QStringLiteral("bbs.jpnkn.com")),
                 BoardType::Jpnkn);
    }

    void detectBoardType_other()
    {
        using namespace yapcr::bbs;
        QCOMPARE(detectBoardType(QStringLiteral("2ch.net")),
                 BoardType::Jpnkn);
    }

    // ======== parseContactUrl — jpnkn ========

    void parse_jpnkn_with_readcgi()
    {
        using namespace yapcr::bbs;
        // bbs.jpnkn.com のサンプル dat: key=1770907315 を使用
        const auto loc = parseContactUrl(
            QStringLiteral("https://bbs.jpnkn.com/test/read.cgi/livegame/1770907315/"));
        QVERIFY(loc.valid);
        QCOMPARE(loc.type,          BoardType::Jpnkn);
        QCOMPARE(loc.board.scheme,  QStringLiteral("https"));
        QCOMPARE(loc.board.host,    QStringLiteral("bbs.jpnkn.com"));
        QCOMPARE(loc.board.base,    QStringLiteral("bbs.jpnkn.com"));
        QCOMPARE(loc.board.board,   QStringLiteral("livegame"));
        QVERIFY2(loc.board.number.isEmpty(), "jpnkn は number フィールドを使わない");
        QCOMPARE(loc.thread.key,    QStringLiteral("1770907315"));
        QCOMPARE(loc.board.code,    Charset::ShiftJis);
    }

    void parse_jpnkn_boardtop()
    {
        using namespace yapcr::bbs;
        // read.cgi なし（板トップ URL）→ key は空で valid
        const auto loc = parseContactUrl(
            QStringLiteral("https://bbs.jpnkn.com/livegame/"));
        QVERIFY(loc.valid);
        QCOMPARE(loc.type,        BoardType::Jpnkn);
        QCOMPARE(loc.board.board, QStringLiteral("livegame"));
        QVERIFY2(loc.thread.key.isEmpty(), "板トップ URL では key は空");
    }

    void parse_jpnkn_https_scheme()
    {
        using namespace yapcr::bbs;
        const auto loc = parseContactUrl(
            QStringLiteral("https://bbs.jpnkn.com/test/read.cgi/anarchy/1234567890/"));
        QVERIFY(loc.valid);
        QCOMPARE(loc.board.scheme, QStringLiteral("https"));
    }

    void parse_jpnkn_ttp_scheme()
    {
        using namespace yapcr::bbs;
        // "ttp://" → scheme = "http"（移植元 makeScheme 相当）
        const auto loc = parseContactUrl(
            QStringLiteral("ttp://bbs.jpnkn.com/test/read.cgi/anarchy/1234567890/"));
        QVERIFY(loc.valid);
        QCOMPARE(loc.board.scheme, QStringLiteral("http"));
    }

    // ======== parseContactUrl — したらば ========

    void parse_shitaraba_with_readcgi()
    {
        using namespace yapcr::bbs;
        const auto loc = parseContactUrl(
            QStringLiteral("https://jbbs.shitaraba.net/bbs/read.cgi/anime/12345/1234567890/"));
        QVERIFY(loc.valid);
        QCOMPARE(loc.type,          BoardType::Shitaraba);
        QCOMPARE(loc.board.scheme,  QStringLiteral("https"));
        QCOMPARE(loc.board.host,    QStringLiteral("jbbs.shitaraba.net"));
        QCOMPARE(loc.board.base,    QStringLiteral("jbbs.shitaraba.net"));
        QCOMPARE(loc.board.board,   QStringLiteral("anime"));
        QCOMPARE(loc.board.number,  QStringLiteral("12345"));
        QCOMPARE(loc.thread.key,    QStringLiteral("1234567890"));
        QCOMPARE(loc.board.code,    Charset::EucJp);
    }

    void parse_shitaraba_boardtop()
    {
        using namespace yapcr::bbs;
        // したらば 板トップ: https://jbbs.shitaraba.net/anime/12345/
        const auto loc = parseContactUrl(
            QStringLiteral("https://jbbs.shitaraba.net/anime/12345/"));
        QVERIFY(loc.valid);
        QCOMPARE(loc.board.board,  QStringLiteral("anime"));
        QCOMPARE(loc.board.number, QStringLiteral("12345"));
        QVERIFY2(loc.thread.key.isEmpty(), "板トップ URL では key は空");
    }

    void parse_invalid()
    {
        using namespace yapcr::bbs;
        const auto loc = parseContactUrl(QStringLiteral("not-a-url"));
        QVERIFY(!loc.valid);
    }

    // ======== URL ビルダー — jpnkn ========

    void urlBuilders_jpnkn()
    {
        using namespace yapcr::bbs;
        const Board b = makeJpnknBoard(
            QStringLiteral("https"),
            QStringLiteral("bbs.jpnkn.com"),
            QStringLiteral("livegame"));
        const QString key = QStringLiteral("1770907315");
        const BoardType t = BoardType::Jpnkn;

        QCOMPARE(boardUrl  (b, t),
                 QStringLiteral("https://bbs.jpnkn.com/livegame/"));
        QCOMPARE(threadUrl (b, key, t),
                 QStringLiteral("https://bbs.jpnkn.com/test/read.cgi/livegame/1770907315/"));
        QCOMPARE(settingUrl(b, t),
                 QStringLiteral("https://bbs.jpnkn.com/livegame/SETTING.TXT"));
        QCOMPARE(subjectUrl(b, t),
                 QStringLiteral("https://bbs.jpnkn.com/livegame/subject.txt"));
        QCOMPARE(datUrl    (b, key, t),
                 QStringLiteral("https://bbs.jpnkn.com/livegame/dat/1770907315.dat"));
        QCOMPARE(writeUrl  (b, t),
                 QStringLiteral("https://bbs.jpnkn.com/test/bbs.cgi"));
    }

    // ======== URL ビルダー — したらば ========

    void urlBuilders_shitaraba()
    {
        using namespace yapcr::bbs;
        const Board b = makeShitarabaBoard(
            QStringLiteral("https"),
            QStringLiteral("jbbs.shitaraba.net"),
            QStringLiteral("anime"),
            QStringLiteral("12345"));
        const QString key = QStringLiteral("1234567890");
        const BoardType t = BoardType::Shitaraba;

        QCOMPARE(boardUrl  (b, t),
                 QStringLiteral("https://jbbs.shitaraba.net/anime/12345/"));
        QCOMPARE(threadUrl (b, key, t),
                 QStringLiteral("https://jbbs.shitaraba.net/bbs/read.cgi/anime/12345/1234567890/"));
        QCOMPARE(settingUrl(b, t),
                 QStringLiteral("https://jbbs.shitaraba.net/bbs/api/setting.cgi/anime/12345/"));
        QCOMPARE(subjectUrl(b, t),
                 QStringLiteral("https://jbbs.shitaraba.net/anime/12345/subject.txt"));
        QCOMPARE(datUrl    (b, key, t),
                 QStringLiteral("https://jbbs.shitaraba.net/bbs/rawmode.cgi/anime/12345/1234567890/"));
        QCOMPARE(writeUrl  (b, t),
                 QStringLiteral("https://jbbs.shitaraba.net/bbs/write.cgi"));
    }

    // ======== 文字コード往復（SJIS） ========

    void charset_sjis_roundtrip()
    {
        using namespace yapcr::bbs;
        // 日本語＋全角記号（アンカー表現にも登場する文字を含む）
        const QString original = QStringLiteral(
            "てすとスレ０１２３ー，＞あいうえお");
        bool ok = false;
        const QByteArray bytes = encodeTo(original, Charset::ShiftJis);
        QVERIFY2(!bytes.isEmpty(), "SJIS エンコードが空");
        const QString decoded = decodeFrom(bytes, Charset::ShiftJis, &ok);
        QVERIFY2(ok, "SJIS デコードにエラーが発生");
        QCOMPARE(decoded, original);
    }

    void charset_eucjp_roundtrip()
    {
        using namespace yapcr::bbs;
        // したらば の EUC-JP 往復確認
        // CP51932 が IsValidCodePage=false な環境（一部 Windows 設定）ではスキップ。
        // M3.8 実機確認で対処する。
        if (!isCharsetSupported(Charset::EucJp)) {
            QSKIP("EUC-JP (CP51932) がこの環境では利用不可 — M3.8 で実機確認");
        }
        const QString original = QStringLiteral(
            "したらばテスト０１２３ー，＞あいうえお");
        bool ok = false;
        const QByteArray bytes = encodeTo(original, Charset::EucJp);
        QVERIFY2(!bytes.isEmpty(), "EUC-JP エンコードが空");
        const QString decoded = decodeFrom(bytes, Charset::EucJp, &ok);
        QVERIFY2(ok, "EUC-JP デコードにエラーが発生");
        QCOMPARE(decoded, original);
    }

    void charset_sjis_known_byte()
    {
        using namespace yapcr::bbs;
        // SJIS "あ" = 0x82 0xA0 — コーデック取り違えを即検出する既知バイト列テスト
        const QByteArray sjis_a = QByteArray::fromHex("82A0");
        bool ok = false;
        const QString decoded = decodeFrom(sjis_a, Charset::ShiftJis, &ok);
        QVERIFY2(ok, "既知 SJIS バイトのデコードにエラー");
        QCOMPARE(decoded, QStringLiteral("あ"));
    }

    void charset_eucjp_known_byte()
    {
        using namespace yapcr::bbs;
        if (!isCharsetSupported(Charset::EucJp)) {
            QSKIP("EUC-JP (CP51932) がこの環境では利用不可 — M3.8 で実機確認");
        }
        // EUC-JP "あ" = 0xA4 0xA2
        const QByteArray euc_a = QByteArray::fromHex("A4A2");
        bool ok = false;
        const QString decoded = decodeFrom(euc_a, Charset::EucJp, &ok);
        QVERIFY2(ok, "既知 EUC-JP バイトのデコードにエラー");
        QCOMPARE(decoded, QStringLiteral("あ"));
    }

    void charset_wrong_codec_mismatch()
    {
        using namespace yapcr::bbs;
        // SJIS でエンコードしたバイトを EUC-JP としてデコードすると
        // 文字列が一致しない（コーデック取り違え検出）
        const QString original = QStringLiteral("テスト");
        const QByteArray sjisBytes = encodeTo(original, Charset::ShiftJis);
        bool ok = false;
        const QString decoded = decodeFrom(sjisBytes, Charset::EucJp, &ok);
        // ok が false になるか、結果が異なることを確認
        // （バイト列が偶然有効な EUC-JP になる可能性は低いが保守的に OR で検証）
        const bool mismatchDetected = !ok || (decoded != original);
        QVERIFY2(mismatchDetected, "SJIS バイトを EUC-JP でデコードしても不一致が検出されなかった");
    }
};

QTEST_MAIN(TstBbs)
#include "tst_bbs.moc"
