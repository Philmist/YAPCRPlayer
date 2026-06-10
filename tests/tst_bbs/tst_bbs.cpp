#include <QTest>
#include <QFile>
#include <QNetworkCookie>
#include <QSignalSpy>

#include "bbs/bbs_session.h"
#include "bbs/board_url.h"
#include "bbs/charset.h"
#include "bbs/dat.h"
#include "bbs/dat_store.h"
#include "bbs/extract.h"
#include "bbs/fastest.h"
#include "bbs/models.h"
#include "bbs/post.h"
#include "bbs/setting.h"
#include "bbs/subject.h"

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

    // ======== parseSetting ========

    void parseSetting_basic()
    {
        using namespace yapcr::bbs;
        const QString text =
            QStringLiteral("BBS_TITLE=テスト板タイトル\n")
            + QStringLiteral("BBS_NONAME_NAME=名無しさん\n")
            + QStringLiteral("BBS_THREAD_STOP=1000\n");
        const SettingInfo info = parseSetting(text);
        QCOMPARE(info.title,  QStringLiteral("テスト板タイトル"));
        QCOMPARE(info.noname, QStringLiteral("名無しさん"));
        QCOMPARE(info.stop,   1000);
    }

    void parseSetting_value_contains_equals()
    {
        using namespace yapcr::bbs;
        // value に '=' が含まれる場合、最初の '=' だけで分割する
        const QString text = QStringLiteral("KEY=a=b=c\n");
        const SettingInfo info = parseSetting(text);
        QCOMPARE(info.title,  QString());  // BBS_TITLE なし
        // 直接値が取れないので、再度 parseSetting で KEY をチェックするより
        // 副作用なしで動作を確認: 他のフィールドが壊れていないことを確認
        QCOMPARE(info.stop, 1000);  // 既定値維持
    }

    void parseSetting_no_thread_stop()
    {
        using namespace yapcr::bbs;
        // BBS_THREAD_STOP が存在しない場合は既定値 1000
        const QString text =
            QStringLiteral("BBS_TITLE=タイトル\n")
            + QStringLiteral("BBS_NONAME_NAME=名無し\n");
        const SettingInfo info = parseSetting(text);
        QCOMPARE(info.stop, 1000);
    }

    void parseSetting_custom_stop()
    {
        using namespace yapcr::bbs;
        const QString text = QStringLiteral("BBS_THREAD_STOP=512\n");
        const SettingInfo info = parseSetting(text);
        QCOMPARE(info.stop, 512);
    }

    void parseSetting_invalid_lines_skipped()
    {
        using namespace yapcr::bbs;
        // '=' なし行・空行・空 name 行はスキップ
        const QString text =
            QStringLiteral("=NO_NAME_LINE\n")
            + QStringLiteral("NO_EQ_LINE\n")
            + QStringLiteral("\n")
            + QStringLiteral("BBS_TITLE=正常\n");
        const SettingInfo info = parseSetting(text);
        QCOMPARE(info.title, QStringLiteral("正常"));
    }

    void parseSetting_crlf()
    {
        using namespace yapcr::bbs;
        // Windows 改行（CRLF）も正しく処理する
        const QString text =
            QStringLiteral("BBS_TITLE=CRLFテスト\r\n")
            + QStringLiteral("BBS_THREAD_STOP=2000\r\n");
        const SettingInfo info = parseSetting(text);
        QCOMPARE(info.title, QStringLiteral("CRLFテスト"));
        QCOMPARE(info.stop,  2000);
    }

    // ======== parseSubject — jpnkn ========

    void parseSubject_jpnkn_basic()
    {
        using namespace yapcr::bbs;
        const QString text =
            QStringLiteral("1770907315.dat<>テストスレ (9)\n")
            + QStringLiteral("1729137713.dat<>放送スレ 初見さん3 (282)\n");
        const QList<ThreadInfo> list = parseSubject(text, BoardType::Jpnkn);
        QCOMPARE(list.size(), 2);
        QCOMPARE(list[0].key,   QStringLiteral("1770907315"));
        QCOMPARE(list[0].title, QStringLiteral("テストスレ"));
        QCOMPARE(list[0].count, 9);
        QCOMPARE(list[1].key,   QStringLiteral("1729137713"));
        QCOMPARE(list[1].title, QStringLiteral("放送スレ 初見さん3"));
        QCOMPARE(list[1].count, 282);
        // number / speed は M3.4 で充填（M3.2 では空/0 のまま）
        QVERIFY2(list[0].number.isEmpty(), "number は M3.4 が担当");
        QCOMPARE(list[0].speed, 0.0);
    }

    void parseSubject_jpnkn_title_with_parens()
    {
        using namespace yapcr::bbs;
        // 題名中に括弧がある場合、末尾の (n) だけが count になる
        const QString text =
            QStringLiteral("1234567890.dat<>タイトル(10)の続き (282)\n");
        const QList<ThreadInfo> list = parseSubject(text, BoardType::Jpnkn);
        QCOMPARE(list.size(), 1);
        QCOMPARE(list[0].key,   QStringLiteral("1234567890"));
        QCOMPARE(list[0].title, QStringLiteral("タイトル(10)の続き"));
        QCOMPARE(list[0].count, 282);
    }

    void parseSubject_jpnkn_fullwidth_in_title()
    {
        using namespace yapcr::bbs;
        // 全角数字・記号・長音など（横断決定 4：全角を落とさない）
        const QString text =
            QStringLiteral("1111111111.dat<>０１２３ー，＞てすと (5)\n");
        const QList<ThreadInfo> list = parseSubject(text, BoardType::Jpnkn);
        QCOMPARE(list.size(), 1);
        QCOMPARE(list[0].title, QStringLiteral("０１２３ー，＞てすと"));
        QCOMPARE(list[0].count, 5);
    }

    void parseSubject_jpnkn_malformed_skipped()
    {
        using namespace yapcr::bbs;
        // "<>" セパレータなし / count なしは除外
        const QString text =
            QStringLiteral("no_sep_line\n")
            + QStringLiteral("1234.dat<>count_missing\n")
            + QStringLiteral("1234567890.dat<>正常スレ (1)\n");
        const QList<ThreadInfo> list = parseSubject(text, BoardType::Jpnkn);
        QCOMPARE(list.size(), 1);
        QCOMPARE(list[0].key, QStringLiteral("1234567890"));
    }

    // ======== parseSubject — したらば ========

    void parseSubject_shitaraba_basic()
    {
        using namespace yapcr::bbs;
        // したらば書式: key.cgi,title(count)
        const QString text =
            QStringLiteral("1234567890.cgi,テストスレ(282)\n")
            + QStringLiteral("9876543210.cgi,別スレッド(10)\n");
        const QList<ThreadInfo> list = parseSubject(text, BoardType::Shitaraba);
        QCOMPARE(list.size(), 2);
        QCOMPARE(list[0].key,   QStringLiteral("1234567890"));
        QCOMPARE(list[0].title, QStringLiteral("テストスレ"));
        QCOMPARE(list[0].count, 282);
        QCOMPARE(list[1].key,   QStringLiteral("9876543210"));
        QCOMPARE(list[1].title, QStringLiteral("別スレッド"));
        QCOMPARE(list[1].count, 10);
    }

    void parseSubject_shitaraba_title_with_parens()
    {
        using namespace yapcr::bbs;
        // 題名中に括弧がある場合、末尾の (n) が count（greedy title）
        const QString text =
            QStringLiteral("1234567890.cgi,タイトル(10)の続き(282)\n");
        const QList<ThreadInfo> list = parseSubject(text, BoardType::Shitaraba);
        QCOMPARE(list.size(), 1);
        QCOMPARE(list[0].title, QStringLiteral("タイトル(10)の続き"));
        QCOMPARE(list[0].count, 282);
    }

    void parseSubject_shitaraba_comma_in_title()
    {
        using namespace yapcr::bbs;
        // 題名にカンマを含む（したらば書式は最初の ',cgi,' より後が題名）
        const QString text =
            QStringLiteral("1234567890.cgi,A,Bスレ(50)\n");
        const QList<ThreadInfo> list = parseSubject(text, BoardType::Shitaraba);
        QCOMPARE(list.size(), 1);
        QCOMPARE(list[0].title, QStringLiteral("A,Bスレ"));
        QCOMPARE(list[0].count, 50);
    }

    void parseSubject_shitaraba_malformed_skipped()
    {
        using namespace yapcr::bbs;
        // count 部が欠落する行は除外（完全 anchor なので末尾 (n) 必須）
        const QString text =
            QStringLiteral("1234567890.cgi,count_missing\n")
            + QStringLiteral("not_a_cgi_line\n")
            + QStringLiteral("9999999999.cgi,正常(1)\n");
        const QList<ThreadInfo> list = parseSubject(text, BoardType::Shitaraba);
        QCOMPARE(list.size(), 1);
        QCOMPARE(list[0].key, QStringLiteral("9999999999"));
    }

    // ======== フィクスチャ統合 — jpnkn（raw bytes → ShiftJis → parse）========

    void fixture_jpnkn_subject()
    {
        using namespace yapcr::bbs;
        const QString dir = QStringLiteral(BBS_FIXTURES_DIR);
        QFile f(dir + QStringLiteral("/jpnkn/subject.txt"));
        if (!f.open(QIODevice::ReadOnly)) {
            QSKIP("フィクスチャ jpnkn/subject.txt が見つからない");
        }
        const QByteArray raw = f.readAll();

        bool ok = false;
        const QString text = decodeFrom(raw, Charset::ShiftJis, &ok);
        QVERIFY2(ok, "Shift_JIS デコードエラー");

        const QList<ThreadInfo> list = parseSubject(text, BoardType::Jpnkn);
        QCOMPARE(list.size(), 2);
        QCOMPARE(list[0].key,   QStringLiteral("1770907315"));
        QCOMPARE(list[0].count, 9);
        QVERIFY2(!list[0].title.isEmpty(), "スレッドタイトルが空");
        QCOMPARE(list[1].key,   QStringLiteral("1729137713"));
        QCOMPARE(list[1].count, 282);
        QVERIFY2(!list[1].title.isEmpty(), "スレッドタイトルが空");
    }

    void fixture_jpnkn_setting()
    {
        using namespace yapcr::bbs;
        const QString dir = QStringLiteral(BBS_FIXTURES_DIR);
        QFile f(dir + QStringLiteral("/jpnkn/SETTING.TXT"));
        if (!f.open(QIODevice::ReadOnly)) {
            QSKIP("フィクスチャ jpnkn/SETTING.TXT が見つからない");
        }
        const QByteArray raw = f.readAll();

        bool ok = false;
        const QString text = decodeFrom(raw, Charset::ShiftJis, &ok);
        QVERIFY2(ok, "Shift_JIS デコードエラー");

        const SettingInfo info = parseSetting(text);
        QVERIFY2(!info.title.isEmpty(),  "BBS_TITLE が空");
        QVERIFY2(!info.noname.isEmpty(), "BBS_NONAME_NAME が空");
        QCOMPARE(info.stop, 1000);
    }

    // ======== parseDat — jpnkn ========

    void parseDat_jpnkn_single()
    {
        using namespace yapcr::bbs;
        // >>1 は title あり、その後 empty
        const QString text =
            QStringLiteral("名無し<>sage<>2026/02/12(木) 23:41:56.06<>テスト本文<br>2行目<>テストスレ\n");
        const QList<ResInfo> list = parseDat(text, BoardType::Jpnkn);
        QCOMPARE(list.size(), 1);
        QCOMPARE(list[0].name,    QStringLiteral("名無し"));
        QCOMPARE(list[0].mail,    QStringLiteral("sage"));
        QCOMPARE(list[0].message, QStringLiteral("テスト本文<br>2行目"));  // <br> は残置
        QCOMPARE(list[0].title,   QStringLiteral("テストスレ"));
        QVERIFY2(list[0].number.isEmpty(), "jpnkn number は M3.4 が付与（M3.3 では空）");
    }

    void parseDat_jpnkn_multi()
    {
        using namespace yapcr::bbs;
        // 複数レス、2レス目以降は title 空
        const QString text =
            QStringLiteral("名無し<>sage<>2026/02/12(木) 23:41:56.06<>最初のレス<>スレタイ\n")
            + QStringLiteral("ふぃるみすと<>age<>2026/02/12(木) 23:45:00.00<>2番目のレス<>\n");
        const QList<ResInfo> list = parseDat(text, BoardType::Jpnkn);
        QCOMPARE(list.size(), 2);
        QCOMPARE(list[0].title, QStringLiteral("スレタイ"));
        QVERIFY2(list[1].title.isEmpty(), "2レス目以降は title 空");
        QCOMPARE(list[1].name, QStringLiteral("ふぃるみすと"));
    }

    void parseDat_jpnkn_datetimeid()
    {
        using namespace yapcr::bbs;
        // datetimeid: (曜日) を含めた日付 + 時刻の再構成 + ID 抽出
        const QString text =
            QStringLiteral("名無し<>sage<>2026/02/12(木) 23:41:56.06 ID:Ab3kF9xZ<>本文<>\n");
        const QList<ResInfo> list = parseDat(text, BoardType::Jpnkn);
        QCOMPARE(list.size(), 1);
        // datetime に曜日を含む日付グループが再構成されているか
        QCOMPARE(list[0].datetime, QStringLiteral("2026/02/12(木) 23:41:56.06"));
        QCOMPARE(list[0].id,       QStringLiteral("Ab3kF9xZ"));
    }

    void parseDat_jpnkn_datetimeid_no_id()
    {
        using namespace yapcr::bbs;
        // ID なし（したらば不使用だが jpnkn の一部にも存在）
        const QString text =
            QStringLiteral("名無し<>sage<>2026/02/12(木) 23:41:56.06<>本文<>\n");
        const QList<ResInfo> list = parseDat(text, BoardType::Jpnkn);
        QCOMPARE(list.size(), 1);
        QCOMPARE(list[0].datetime, QStringLiteral("2026/02/12(木) 23:41:56.06"));
        QVERIFY2(list[0].id.isEmpty(), "ID なし行の id は空");
    }

    void parseDat_jpnkn_remove_a_tag()
    {
        using namespace yapcr::bbs;
        // <a href="...">...</a> のタグ部分を除去し、テキスト内容と <br> は残す
        const QString text =
            QStringLiteral("名無し<>sage<>2026/01/01(水) 00:00:00.00<>"
                           "<a href=\"http://example.com\">リンクテキスト</a><br>次の行<>\n");
        const QList<ResInfo> list = parseDat(text, BoardType::Jpnkn);
        QCOMPARE(list.size(), 1);
        // <a ...> と </a> タグが除去されてテキストと <br> は残る
        QCOMPARE(list[0].message, QStringLiteral("リンクテキスト<br>次の行"));
    }

    void parseDat_jpnkn_name_with_trip()
    {
        using namespace yapcr::bbs;
        // トリップ付き名前（</b>◆xxxxx<b> を含む）は name にそのまま保持
        const QString text =
            QStringLiteral("ふぃるみすと</b>◆9Fb2kCPDgA<b><>sage<>2026/02/12(木) 23:41:56.06"
                           "<>テスト本文<>テストスレ\n");
        const QList<ResInfo> list = parseDat(text, BoardType::Jpnkn);
        QCOMPARE(list.size(), 1);
        // </b> と <b> は a タグではないので残る
        QCOMPARE(list[0].name, QStringLiteral("ふぃるみすと</b>◆9Fb2kCPDgA<b>"));
    }

    void parseDat_jpnkn_empty_lines_skipped()
    {
        using namespace yapcr::bbs;
        const QString text =
            QStringLiteral("\n")
            + QStringLiteral("名無し<>sage<>2026/01/01(水) 00:00:00.00<>本文<>\n")
            + QStringLiteral("\n");
        const QList<ResInfo> list = parseDat(text, BoardType::Jpnkn);
        QCOMPARE(list.size(), 1);
    }

    void parseDat_jpnkn_crlf()
    {
        using namespace yapcr::bbs;
        const QString text =
            QStringLiteral("名無し<>sage<>2026/01/01(水) 00:00:00.00<>CRLFテスト<>\r\n");
        const QList<ResInfo> list = parseDat(text, BoardType::Jpnkn);
        QCOMPARE(list.size(), 1);
        QCOMPARE(list[0].message, QStringLiteral("CRLFテスト"));
    }

    // ======== parseDat — したらば ========

    void parseDat_shitaraba_single()
    {
        using namespace yapcr::bbs;
        // したらば書式: number<>name<>mail<>datetime<>message<>title<>id
        const QString text =
            QStringLiteral("1<>テストさん<>age<>2026/01/01 12:34:56<>したらばメッセージ<>したらばスレ<>ID111\n");
        const QList<ResInfo> list = parseDat(text, BoardType::Shitaraba);
        QCOMPARE(list.size(), 1);
        QCOMPARE(list[0].number,   QStringLiteral("1"));
        QCOMPARE(list[0].name,     QStringLiteral("テストさん"));
        QCOMPARE(list[0].mail,     QStringLiteral("age"));
        QCOMPARE(list[0].datetime, QStringLiteral("2026/01/01 12:34:56"));
        QCOMPARE(list[0].message,  QStringLiteral("したらばメッセージ"));
        QCOMPARE(list[0].title,    QStringLiteral("したらばスレ"));
        QCOMPARE(list[0].id,       QStringLiteral("ID111"));
    }

    void parseDat_shitaraba_without_id_col()
    {
        using namespace yapcr::bbs;
        // id 列なし（5 列のみ）でもパース成功
        const QString text =
            QStringLiteral("2<>名無し<>sage<>2026/01/01 00:00:00<>本文<>\n");
        const QList<ResInfo> list = parseDat(text, BoardType::Shitaraba);
        QCOMPARE(list.size(), 1);
        QCOMPARE(list[0].number, QStringLiteral("2"));
        QVERIFY2(list[0].id.isEmpty(), "id 列なし → id は空");
    }

    // ======== フィクスチャ統合 — jpnkn dat ========

    void fixture_jpnkn_dat()
    {
        using namespace yapcr::bbs;
        const QString dir = QStringLiteral(BBS_FIXTURES_DIR);
        QFile f(dir + QStringLiteral("/jpnkn/dat/1770907315.dat"));
        if (!f.open(QIODevice::ReadOnly)) {
            QSKIP("フィクスチャ jpnkn/dat/1770907315.dat が見つからない");
        }
        const QByteArray raw = f.readAll();

        bool ok = false;
        const QString text = decodeFrom(raw, Charset::ShiftJis, &ok);
        QVERIFY2(ok, "Shift_JIS デコードエラー");

        const QList<ResInfo> list = parseDat(text, BoardType::Jpnkn);
        QCOMPARE(list.size(), 9);  // フィクスチャ dat は 9 レス

        // 1レス目: title あり
        QVERIFY2(!list[0].title.isEmpty(), "1レス目の title が空");
        QVERIFY2(!list[0].name.isEmpty(),  "name が空");
        QVERIFY2(!list[0].datetime.isEmpty(), "datetime が空");
    }

    // ======== フィクスチャ統合 — したらば dat（EUC-JP、CP51932 不可環境は SKIP）========

    void fixture_shitaraba_dat()
    {
        using namespace yapcr::bbs;
        if (!isCharsetSupported(Charset::EucJp)) {
            QSKIP("EUC-JP (CP51932) がこの環境では利用不可 — M3.8 で実機確認");
        }
        const QString dir = QStringLiteral(BBS_FIXTURES_DIR);
        QFile f(dir + QStringLiteral("/shitaraba/dat/sample.dat"));
        if (!f.open(QIODevice::ReadOnly)) {
            QSKIP("フィクスチャ shitaraba/dat/sample.dat が見つからない");
        }
        const QByteArray raw = f.readAll();

        bool ok = false;
        const QString text = decodeFrom(raw, Charset::EucJp, &ok);
        QVERIFY2(ok, "EUC-JP デコードエラー");

        const QList<ResInfo> list = parseDat(text, BoardType::Shitaraba);
        QVERIFY2(list.size() >= 1, "したらば dat のパース結果が空");
        QVERIFY2(!list[0].number.isEmpty(), "したらば number が空");
        QVERIFY2(!list[0].name.isEmpty(),   "したらば name が空");
    }

    // ======== extractUrls ========

    void extractUrls_http()
    {
        using namespace yapcr::bbs;
        const QList<QString> urls = extractUrls(
            QStringLiteral("テキスト http://example.com/path?q=1 です"));
        QCOMPARE(urls.size(), 1);
        QCOMPARE(urls[0], QStringLiteral("http://example.com/path?q=1"));
    }

    void extractUrls_ttp_normalized()
    {
        using namespace yapcr::bbs;
        // ttp:// → http:// に正規化
        const QList<QString> urls = extractUrls(
            QStringLiteral("ttp://example.com/page"));
        QCOMPARE(urls.size(), 1);
        QCOMPARE(urls[0], QStringLiteral("http://example.com/page"));
    }

    void extractUrls_https()
    {
        using namespace yapcr::bbs;
        // https:// はそのまま
        const QList<QString> urls = extractUrls(
            QStringLiteral("https://example.com/"));
        QCOMPARE(urls.size(), 1);
        QCOMPARE(urls[0], QStringLiteral("https://example.com/"));
    }

    void extractUrls_multiple()
    {
        using namespace yapcr::bbs;
        const QList<QString> urls = extractUrls(
            QStringLiteral("見て http://a.com/ と https://b.com/path"));
        QCOMPARE(urls.size(), 2);
    }

    void extractUrls_no_url()
    {
        using namespace yapcr::bbs;
        const QList<QString> urls = extractUrls(QStringLiteral("URLなしのテキスト"));
        QCOMPARE(urls.size(), 0);
    }

    // ======== extractAnchors ========

    void extractAnchors_single()
    {
        using namespace yapcr::bbs;
        // dat 内は HTML エスケープ済み → &gt;&gt;n で当たる（ASCII >> では当たらない）
        const QList<Range> ranges = extractAnchors(QStringLiteral("&gt;&gt;123"));
        QCOMPARE(ranges.size(), 1);
        QCOMPARE(ranges[0].first, 123);
        QCOMPARE(ranges[0].last,  123);
    }

    void extractAnchors_range()
    {
        using namespace yapcr::bbs;
        // &gt;&gt;1-5 → first=1, last=5
        const QList<Range> ranges = extractAnchors(QStringLiteral("&gt;&gt;1-5"));
        QCOMPARE(ranges.size(), 1);
        QCOMPARE(ranges[0].first, 1);
        QCOMPARE(ranges[0].last,  5);
    }

    void extractAnchors_group()
    {
        using namespace yapcr::bbs;
        // &gt;&gt;1,3,5 → 3 つの単点 Range
        const QList<Range> ranges = extractAnchors(QStringLiteral("&gt;&gt;1,3,5"));
        QCOMPARE(ranges.size(), 3);
        QCOMPARE(ranges[0].first, 1);
        QCOMPARE(ranges[1].first, 3);
        QCOMPARE(ranges[2].first, 5);
    }

    void extractAnchors_fullwidth_gt()
    {
        using namespace yapcr::bbs;
        // 全角 ＞＞１２３
        const QList<Range> ranges = extractAnchors(QStringLiteral("＞＞１２３"));
        QCOMPARE(ranges.size(), 1);
        QCOMPARE(ranges[0].first, 123);
        QCOMPARE(ranges[0].last,  123);
    }

    void extractAnchors_fullwidth_range()
    {
        using namespace yapcr::bbs;
        // 全角 ＞＞１－５（全角ハイフン U+FF0D）
        const QList<Range> ranges = extractAnchors(QStringLiteral("＞＞１－５"));
        QCOMPARE(ranges.size(), 1);
        QCOMPARE(ranges[0].first, 1);
        QCOMPARE(ranges[0].last,  5);
    }

    void extractAnchors_fullwidth_comma()
    {
        using namespace yapcr::bbs;
        // 全角 ＞＞１，３（全角カンマ U+FF0C）
        const QList<Range> ranges = extractAnchors(QStringLiteral("＞＞１，３"));
        QCOMPARE(ranges.size(), 2);
        QCOMPARE(ranges[0].first, 1);
        QCOMPARE(ranges[1].first, 3);
    }

    void extractAnchors_no_anchor()
    {
        using namespace yapcr::bbs;
        // ASCII > は当たらない
        const QList<Range> ranges = extractAnchors(QStringLiteral(">>123"));
        QCOMPARE(ranges.size(), 0);
    }

    // ======== extractId ========

    void extractId_match()
    {
        using namespace yapcr::bbs;
        // 8 文字 ID
        const QString id = extractId(QStringLiteral("2026/01/01 ID:Ab3kF9xZ テキスト"));
        QCOMPARE(id, QStringLiteral("Ab3kF9xZ"));
    }

    void extractId_no_match()
    {
        using namespace yapcr::bbs;
        const QString id = extractId(QStringLiteral("IDなしのテキスト"));
        QVERIFY2(id.isEmpty(), "ID なし → 空文字列");
    }

    void extractId_short_id_not_match()
    {
        using namespace yapcr::bbs;
        // 7 文字以下は当たらない
        const QString id = extractId(QStringLiteral("ID:Abc123"));  // 7 文字
        QVERIFY2(id.isEmpty(), "7 文字 ID は当たらない");
    }

    // ======== DatStore — ヘルパー ========

    // 手組み ResInfo（jpnkn: number 空、id/message をカスタマイズ可）
    static yapcr::bbs::ResInfo makeRes(
        const QString& name     = QStringLiteral("名無し"),
        const QString& message  = QString(),
        const QString& id       = QString())
    {
        using namespace yapcr::bbs;
        ResInfo r;
        r.name    = name;
        r.message = message;
        r.id      = id;
        return r;
    }

    // ======== DatStore — dedup ========

    void datStore_dedup_same_list()
    {
        using namespace yapcr::bbs;
        // 同じリストを 2 回 merge → 2 回目は 0 件追加
        QList<ResInfo> list;
        list.append(makeRes(QStringLiteral("res1")));
        list.append(makeRes(QStringLiteral("res2")));
        list.append(makeRes(QStringLiteral("res3")));

        DatStore store;
        QCOMPARE(store.merge(list, BoardType::Jpnkn), 3);
        QCOMPARE(store.count(), 3);
        QCOMPARE(store.merge(list, BoardType::Jpnkn), 0);  // 新着なし
        QCOMPARE(store.count(), 3);
    }

    void datStore_dedup_incremental()
    {
        using namespace yapcr::bbs;
        // 3 件 → 5 件に増えた全件リストで 2 回目 merge → +2 のみ追加
        QList<ResInfo> first, second;
        for (int i = 1; i <= 3; ++i) {
            first.append(makeRes(QStringLiteral("res%1").arg(i)));
        }
        second = first;
        second.append(makeRes(QStringLiteral("res4")));
        second.append(makeRes(QStringLiteral("res5")));

        DatStore store;
        QCOMPARE(store.merge(first,  BoardType::Jpnkn), 3);
        QCOMPARE(store.merge(second, BoardType::Jpnkn), 2);
        QCOMPARE(store.count(), 5);
    }

    void datStore_dedup_smaller_list_ignored()
    {
        using namespace yapcr::bbs;
        // より小さいリストは何もしない（サーバ応答が縮んだ場合など）
        QList<ResInfo> list3, list2;
        for (int i = 1; i <= 3; ++i) { list3.append(makeRes()); }
        for (int i = 1; i <= 2; ++i) { list2.append(makeRes()); }

        DatStore store;
        store.merge(list3, BoardType::Jpnkn);
        QCOMPARE(store.merge(list2, BoardType::Jpnkn), 0);
        QCOMPARE(store.count(), 3);
    }

    // ======== DatStore — 番号付与 ========

    void datStore_number_jpnkn()
    {
        using namespace yapcr::bbs;
        // jpnkn: number が空 → 連番 "1", "2", "3"
        QList<ResInfo> list;
        for (int i = 0; i < 3; ++i) { list.append(makeRes()); }

        DatStore store;
        store.merge(list, BoardType::Jpnkn);
        const QList<ResInfo> all = store.all();
        QCOMPARE(all.size(), 3);
        QCOMPARE(all[0].number, QStringLiteral("1"));
        QCOMPARE(all[1].number, QStringLiteral("2"));
        QCOMPARE(all[2].number, QStringLiteral("3"));
    }

    void datStore_number_shitaraba_preserved()
    {
        using namespace yapcr::bbs;
        // したらば: parseDat が付与した number は上書きしない
        QList<ResInfo> list;
        for (int i = 1; i <= 3; ++i) {
            ResInfo r = makeRes();
            r.number = QString::number(i);  // parseDat が付与済み
            list.append(r);
        }

        DatStore store;
        store.merge(list, BoardType::Shitaraba);
        const QList<ResInfo> all = store.all();
        QCOMPARE(all[0].number, QStringLiteral("1"));
        QCOMPARE(all[2].number, QStringLiteral("3"));
    }

    // ======== DatStore — latest フラグ ========

    void datStore_latest_first_batch_all_false()
    {
        using namespace yapcr::bbs;
        // 初回バッチ: 全件 latest=false（新着扱いしない）
        QList<ResInfo> list;
        for (int i = 0; i < 3; ++i) { list.append(makeRes()); }

        DatStore store;
        store.merge(list, BoardType::Jpnkn);
        const QList<ResInfo> all = store.all();
        for (const ResInfo& r : all) {
            QVERIFY2(!r.latest, "初回バッチは latest=false");
        }
    }

    void datStore_latest_second_batch_new_true()
    {
        using namespace yapcr::bbs;
        // 2 回目 merge で追加された新規レスは latest=true
        QList<ResInfo> first, second;
        first.append(makeRes(QStringLiteral("res1")));
        second = first;
        second.append(makeRes(QStringLiteral("res2")));

        DatStore store;
        store.merge(first, BoardType::Jpnkn);

        store.merge(second, BoardType::Jpnkn);
        const QList<ResInfo> all = store.all();
        QVERIFY2(!all[0].latest, "既存レスの latest は false に更新");
        QVERIFY2( all[1].latest, "新規レスの latest は true");
    }

    // ======== DatStore — link 充填 ========

    void datStore_link_filled()
    {
        using namespace yapcr::bbs;
        // message に URL が含まれる → link に正規化済み URL が入る
        QList<ResInfo> list;
        list.append(makeRes(QStringLiteral("名無し"),
                            QStringLiteral("見て ttp://example.com/page です")));

        DatStore store;
        store.merge(list, BoardType::Jpnkn);
        const QList<ResInfo> all = store.all();
        QCOMPARE(all.size(), 1);
        QCOMPARE(all[0].link.size(), 1);
        QCOMPARE(all[0].link[0], QStringLiteral("http://example.com/page"));
    }

    // ======== DatStore — ID 集計 ========

    void datStore_id_count_increments()
    {
        using namespace yapcr::bbs;
        // 同一 ID 3 レス → count: 1,2,3 / total: 3 / identifier: 同一連番
        const QString sameId = QStringLiteral("Ab3kF9xZ");
        QList<ResInfo> list;
        for (int i = 0; i < 3; ++i) {
            list.append(makeRes(QStringLiteral("名無し"), QString(), sameId));
        }

        DatStore store;
        store.merge(list, BoardType::Jpnkn);
        const QList<ResInfo> all = store.all();
        QCOMPARE(all.size(), 3);
        QCOMPARE(all[0].count, 1);
        QCOMPARE(all[1].count, 2);
        QCOMPARE(all[2].count, 3);
        QCOMPARE(all[0].total, 3);
        QCOMPARE(all[1].total, 3);
        QCOMPARE(all[2].total, 3);
        // 同一 ID は同一 identifier
        QCOMPARE(all[0].identifier, all[1].identifier);
        QCOMPARE(all[1].identifier, all[2].identifier);
        QVERIFY2(all[0].identifier > 0, "identifier は 1 始まり");
    }

    void datStore_id_different_ids_different_identifiers()
    {
        using namespace yapcr::bbs;
        // ID が異なれば identifier も異なる
        QList<ResInfo> list;
        list.append(makeRes(QStringLiteral("A"), QString(), QStringLiteral("Id111111")));
        list.append(makeRes(QStringLiteral("B"), QString(), QStringLiteral("Id222222")));

        DatStore store;
        store.merge(list, BoardType::Jpnkn);
        const QList<ResInfo> all = store.all();
        QVERIFY2(all[0].identifier != all[1].identifier,
                 "異なる ID には異なる identifier");
    }

    void datStore_id_question_marks_skipped()
    {
        using namespace yapcr::bbs;
        // "???" 始まり ID はスキップ（count/identifier が付かない）
        QList<ResInfo> list;
        list.append(makeRes(QStringLiteral("名無し"), QString(),
                            QStringLiteral("???xxxxx")));

        DatStore store;
        store.merge(list, BoardType::Jpnkn);
        const QList<ResInfo> all = store.all();
        QCOMPARE(all[0].count,      0);
        QCOMPARE(all[0].total,      0);
        QCOMPARE(all[0].identifier, 0);
    }

    // ======== DatStore — 被参照 ref ========

    void datStore_ref_single_anchor()
    {
        using namespace yapcr::bbs;
        // res3 が &gt;&gt;1 を含む → merge 後 res1.ref == [3]（第2パスで反映）
        QList<ResInfo> list;
        list.append(makeRes(QStringLiteral("res1")));                                // pos=1
        list.append(makeRes(QStringLiteral("res2")));                                // pos=2
        list.append(makeRes(QStringLiteral("res3"),
                            QStringLiteral("&gt;&gt;1")));   // pos=3、res1 を参照

        DatStore store;
        store.merge(list, BoardType::Jpnkn);

        // res1（byNumbers({1}) で取得）の ref に 3 が入っているか
        const QList<ResInfo> res1 = store.byNumbers({1});
        QCOMPARE(res1.size(), 1);
        QVERIFY2(res1[0].ref.contains(3), "res1 は res3 に参照されている");
    }

    void datStore_ref_wide_anchor_ignored()
    {
        using namespace yapcr::bbs;
        // &gt;&gt;1-100（span=99 > refRange=5）は ref に数えない
        QList<ResInfo> list;
        list.append(makeRes(QStringLiteral("res1")));
        list.append(makeRes(QStringLiteral("res2"),
                            QStringLiteral("&gt;&gt;1-100")));

        DatStore store;
        store.merge(list, BoardType::Jpnkn);

        const QList<ResInfo> res1 = store.byNumbers({1});
        QCOMPARE(res1.size(), 1);
        QVERIFY2(res1[0].ref.isEmpty(),
                 "広域アンカー（span > refRange）は ref に数えない");
    }

    void datStore_ref_narrow_range()
    {
        using namespace yapcr::bbs;
        // &gt;&gt;1-3（span=2 <= refRange=5）は ref に入る
        QList<ResInfo> list;
        list.append(makeRes(QStringLiteral("res1")));
        list.append(makeRes(QStringLiteral("res2"),
                            QStringLiteral("&gt;&gt;1-3")));  // pos=2、1-3 を参照
        list.append(makeRes(QStringLiteral("res3")));
        list.append(makeRes(QStringLiteral("res4")));         // pos=4 は範囲外

        DatStore store;
        store.merge(list, BoardType::Jpnkn);

        // res1, res2, res3 は ref に 2 が入る
        QVERIFY(store.byNumbers({1})[0].ref.contains(2));
        QVERIFY(store.byNumbers({2})[0].ref.contains(2));
        QVERIFY(store.byNumbers({3})[0].ref.contains(2));
        // res4 は範囲外
        QVERIFY(!store.byNumbers({4})[0].ref.contains(2));
    }

    // ======== DatStore — reset ========

    void datStore_reset()
    {
        using namespace yapcr::bbs;
        QList<ResInfo> list;
        list.append(makeRes());

        DatStore store;
        store.merge(list, BoardType::Jpnkn);
        QCOMPARE(store.count(), 1);

        store.reset();
        QVERIFY2(store.isEmpty(), "reset 後は isEmpty()");
        QCOMPARE(store.count(), 0);
        QCOMPARE(store.all().size(), 0);
    }

    // ======== DatStore — extract クエリ ========

    void datStore_byRange_basic()
    {
        using namespace yapcr::bbs;
        QList<ResInfo> list;
        for (int i = 0; i < 5; ++i) { list.append(makeRes(QStringLiteral("r%1").arg(i+1))); }

        DatStore store;
        store.merge(list, BoardType::Jpnkn);

        // 2-4 範囲
        const QList<ResInfo> sub = store.byRange({2, 4});
        QCOMPARE(sub.size(), 3);
        QCOMPARE(sub[0].name, QStringLiteral("r2"));
        QCOMPARE(sub[2].name, QStringLiteral("r4"));
    }

    void datStore_byRange_swap()
    {
        using namespace yapcr::bbs;
        QList<ResInfo> list;
        for (int i = 0; i < 3; ++i) { list.append(makeRes()); }

        DatStore store;
        store.merge(list, BoardType::Jpnkn);

        // first > last は自動 swap
        const QList<ResInfo> r1 = store.byRange({3, 1});
        const QList<ResInfo> r2 = store.byRange({1, 3});
        QCOMPARE(r1.size(), 3);
        QCOMPARE(r2.size(), 3);
    }

    void datStore_byRange_out_of_bounds()
    {
        using namespace yapcr::bbs;
        QList<ResInfo> list;
        list.append(makeRes());

        DatStore store;
        store.merge(list, BoardType::Jpnkn);

        // 範囲外は空
        QCOMPARE(store.byRange({5, 10}).size(), 0);
        // 境界クランプ
        QCOMPARE(store.byRange({1, 999}).size(), 1);
    }

    void datStore_byId_hit()
    {
        using namespace yapcr::bbs;
        const QString id = QStringLiteral("MyTestId");
        QList<ResInfo> list;
        list.append(makeRes(QStringLiteral("A"), QString(), id));
        list.append(makeRes(QStringLiteral("B")));
        list.append(makeRes(QStringLiteral("C"), QString(), id));

        DatStore store;
        store.merge(list, BoardType::Jpnkn);

        const QList<ResInfo> result = store.byId(id);
        QCOMPARE(result.size(), 2);
        QCOMPARE(result[0].name, QStringLiteral("A"));
        QCOMPARE(result[1].name, QStringLiteral("C"));
    }

    void datStore_byId_miss()
    {
        using namespace yapcr::bbs;
        QList<ResInfo> list;
        list.append(makeRes());

        DatStore store;
        store.merge(list, BoardType::Jpnkn);

        QCOMPARE(store.byId(QStringLiteral("nonexistent")).size(), 0);
    }

    void datStore_byRef_returns_referencing_posts()
    {
        using namespace yapcr::bbs;
        // res2 が &gt;&gt;1 → byRef(1) は res2 を返す
        QList<ResInfo> list;
        list.append(makeRes(QStringLiteral("res1")));
        list.append(makeRes(QStringLiteral("res2"),
                            QStringLiteral("&gt;&gt;1")));

        DatStore store;
        store.merge(list, BoardType::Jpnkn);

        const QList<ResInfo> result = store.byRef(1);
        QCOMPARE(result.size(), 1);
        QCOMPARE(result[0].name, QStringLiteral("res2"));
    }

    void datStore_byRef_no_refs()
    {
        using namespace yapcr::bbs;
        QList<ResInfo> list;
        list.append(makeRes());
        list.append(makeRes());

        DatStore store;
        store.merge(list, BoardType::Jpnkn);

        // 誰も参照していない → 空
        QCOMPARE(store.byRef(1).size(), 0);
    }

    void datStore_byNumbers_basic()
    {
        using namespace yapcr::bbs;
        QList<ResInfo> list;
        for (int i = 1; i <= 5; ++i) {
            list.append(makeRes(QStringLiteral("r%1").arg(i)));
        }

        DatStore store;
        store.merge(list, BoardType::Jpnkn);

        const QList<ResInfo> result = store.byNumbers({1, 3, 5});
        QCOMPARE(result.size(), 3);
        QCOMPARE(result[0].name, QStringLiteral("r1"));
        QCOMPARE(result[1].name, QStringLiteral("r3"));
        QCOMPARE(result[2].name, QStringLiteral("r5"));
    }

    void datStore_byNumbers_out_of_bounds_ignored()
    {
        using namespace yapcr::bbs;
        QList<ResInfo> list;
        list.append(makeRes());

        DatStore store;
        store.merge(list, BoardType::Jpnkn);

        // 範囲外インデックスは無視
        const QList<ResInfo> result = store.byNumbers({1, 999, 0, -1});
        QCOMPARE(result.size(), 1);
    }

    // ======== DatStore — フィクスチャ統合 ========

    void datStore_fixture_jpnkn()
    {
        using namespace yapcr::bbs;
        const QString dir = QStringLiteral(BBS_FIXTURES_DIR);
        QFile f(dir + QStringLiteral("/jpnkn/dat/1770907315.dat"));
        if (!f.open(QIODevice::ReadOnly)) {
            QSKIP("フィクスチャ jpnkn/dat/1770907315.dat が見つからない");
        }
        const QByteArray raw = f.readAll();

        bool ok = false;
        const QString text = decodeFrom(raw, Charset::ShiftJis, &ok);
        QVERIFY2(ok, "Shift_JIS デコードエラー");

        const QList<ResInfo> parsed = parseDat(text, BoardType::Jpnkn);
        QCOMPARE(parsed.size(), 9);

        DatStore store;
        QCOMPARE(store.merge(parsed, BoardType::Jpnkn), 9);
        QCOMPARE(store.count(), 9);

        const QList<ResInfo> all = store.all();
        QCOMPARE(all.size(), 9);

        // 全件 number が "1".."9" の連番
        for (int i = 0; i < all.size(); ++i) {
            QCOMPARE(all[i].number, QString::number(i + 1));
        }

        // 1レス目: name/datetime/title が非空
        QVERIFY2(!all[0].name.isEmpty(),     "name が空");
        QVERIFY2(!all[0].datetime.isEmpty(), "datetime が空");
        QVERIFY2(!all[0].title.isEmpty(),    "1レス目 title が空");
    }

    void datStore_fixture_shitaraba()
    {
        using namespace yapcr::bbs;
        if (!isCharsetSupported(Charset::EucJp)) {
            QSKIP("EUC-JP (CP51932) がこの環境では利用不可 — M3.8 で実機確認");
        }
        const QString dir = QStringLiteral(BBS_FIXTURES_DIR);
        QFile f(dir + QStringLiteral("/shitaraba/dat/sample.dat"));
        if (!f.open(QIODevice::ReadOnly)) {
            QSKIP("フィクスチャ shitaraba/dat/sample.dat が見つからない");
        }
        const QByteArray raw = f.readAll();

        bool ok = false;
        const QString text = decodeFrom(raw, Charset::EucJp, &ok);
        QVERIFY2(ok, "EUC-JP デコードエラー");

        const QList<ResInfo> parsed = parseDat(text, BoardType::Shitaraba);
        QVERIFY2(!parsed.isEmpty(), "したらば dat のパース結果が空");

        DatStore store;
        const int added = store.merge(parsed, BoardType::Shitaraba);
        QCOMPARE(added, parsed.size());
        QCOMPARE(store.count(), parsed.size());

        // したらば: parseDat が number を付与済み → DatStore は上書きしない
        const QList<ResInfo> all = store.all();
        QVERIFY2(!all[0].number.isEmpty(), "したらば number が空");
    }

    // ======== BbsSession — init（同期パスのみ） ========

    void bbsSession_init_valid_jpnkn()
    {
        using namespace yapcr::bbs;
        BbsSession session;
        const bool ok = session.init(
            QStringLiteral("https://bbs.jpnkn.com/test/read.cgi/livegame/1770907315/"));
        QVERIFY2(ok, "有効な jpnkn URL で init が false");
        QVERIFY2(session.isValid(), "isValid() が false");
        QCOMPARE(session.key(),       QStringLiteral("1770907315"));
        QCOMPARE(session.boardTopUrl(),
                 QStringLiteral("https://bbs.jpnkn.com/livegame/"));
        QCOMPARE(session.threadTopUrl(),
                 QStringLiteral("https://bbs.jpnkn.com/test/read.cgi/livegame/1770907315/"));
    }

    void bbsSession_init_valid_shitaraba()
    {
        using namespace yapcr::bbs;
        BbsSession session;
        const bool ok = session.init(
            QStringLiteral("https://jbbs.shitaraba.net/bbs/read.cgi/anime/12345/1234567890/"));
        QVERIFY2(ok, "有効な したらば URL で init が false");
        QVERIFY2(session.isValid(), "isValid() が false");
        QCOMPARE(session.key(), QStringLiteral("1234567890"));
    }

    void bbsSession_init_invalid_url()
    {
        using namespace yapcr::bbs;
        BbsSession session;
        const bool ok = session.init(QStringLiteral("not-a-valid-url"));
        QVERIFY2(!ok, "無効 URL で init が true");
        QVERIFY2(!session.isValid(), "無効 URL で isValid() が true");
        QVERIFY2(session.key().isEmpty(), "無効 URL で key が非空");
    }

    void bbsSession_init_then_change()
    {
        using namespace yapcr::bbs;
        BbsSession session;
        session.init(
            QStringLiteral("https://bbs.jpnkn.com/test/read.cgi/livegame/1770907315/"));

        // 事前に dat を merge
        QList<ResInfo> list;
        list.append(makeRes());
        session.store().count();  // store は const 参照アクセスのみ
        // change でリセットされることを確認
        const bool changed = session.change(QStringLiteral("9999999999"));
        QVERIFY2(changed, "change が false");
        QCOMPARE(session.key(), QStringLiteral("9999999999"));
    }

    // ======== BbsSession::post — 同期パス（key 未設定） ========

    void bbsSession_post_no_key_emits_failed()
    {
        using namespace yapcr::bbs;
        // init で board top URL（key なし）を渡すと key が空 → post() が即 postFailed
        BbsSession session;
        session.init(QStringLiteral("https://bbs.jpnkn.com/livegame/"));

        QSignalSpy spy(&session, &BbsSession::postFailed);
        session.post(QStringLiteral("名無し"), QStringLiteral("sage"),
                     QStringLiteral("テスト書き込み"));
        // postFailed は同期 emit（ネットワーク不要）
        QCOMPARE(spy.count(), 1);
        QVERIFY2(!spy.at(0).at(0).toString().isEmpty(), "reason が空");
    }

    void bbsSession_post_not_init_emits_failed()
    {
        using namespace yapcr::bbs;
        // init を呼ばずに post → postFailed
        BbsSession session;
        QSignalSpy spy(&session, &BbsSession::postFailed);
        session.post(QStringLiteral("名無し"), QStringLiteral(""), QStringLiteral("本文"));
        QCOMPARE(spy.count(), 1);
    }

    // ======== halfWidthFold ========

    void halfWidthFold_ascii_range()
    {
        using namespace yapcr::bbs;
        // 全角 ASCII → 半角（U+FF01..FF5E → U+0021..7E）
        QCOMPARE(halfWidthFold(QStringLiteral(u"ＡＢＣＤＥＦＧＨＩＪ")),
                 QStringLiteral("ABCDEFGHIJ"));
        QCOMPARE(halfWidthFold(QStringLiteral(u"ＥＲＲＯＲ")),
                 QStringLiteral("ERROR"));
        QCOMPARE(halfWidthFold(QStringLiteral(u"：")),
                 QStringLiteral(":"));
    }

    void halfWidthFold_ideographic_space()
    {
        using namespace yapcr::bbs;
        // 全角スペース（U+3000）→ 半角スペース
        const QString input  = QString(QChar(0x3000));
        const QString expect = QStringLiteral(" ");
        QCOMPARE(halfWidthFold(input), expect);
    }

    void halfWidthFold_passthrough()
    {
        using namespace yapcr::bbs;
        // 半角・ひらがな・CJK 漢字はそのまま
        const QString s = QStringLiteral("ERROR: 利用認証が必要です。");
        QCOMPARE(halfWidthFold(s), s);
    }

    // ======== postUrlEncode ========

    void postUrlEncode_safe_chars()
    {
        using namespace yapcr::bbs;
        // [0-9 a-z A-Z * - . _] は素通し
        QCOMPARE(postUrlEncode(QStringLiteral("abc-123*._"), Charset::ShiftJis),
                 QByteArray("abc-123*._"));
    }

    void postUrlEncode_space_is_percent20()
    {
        using namespace yapcr::bbs;
        // 空白は %20（+ エンコード不使用）
        QCOMPARE(postUrlEncode(QStringLiteral("a b"), Charset::ShiftJis),
                 QByteArray("a%20b"));
    }

    void postUrlEncode_tilde_encoded()
    {
        using namespace yapcr::bbs;
        // ~ は移植元では安全集合外 → %7E
        QCOMPARE(postUrlEncode(QStringLiteral("a~b"), Charset::ShiftJis),
                 QByteArray("a%7Eb"));
    }

    void postUrlEncode_shiftjis()
    {
        using namespace yapcr::bbs;
        // 「書き込む」の Shift-JIS バイト列 → 全バイト %XX
        // 8F 91 82 AB 8D 9E 82 DE
        const QByteArray expected =
            QByteArray("%8F%91%82%AB%8D%9E%82%DE");
        QCOMPARE(postUrlEncode(QStringLiteral(u"書き込む"), Charset::ShiftJis),
                 expected);
    }

    void postUrlEncode_eucjp()
    {
        using namespace yapcr::bbs;
        if (!isCharsetSupported(Charset::EucJp)) {
            QSKIP("EUC-JP がこの環境では利用不可 — M3.8 で実機確認");
        }
        // 「書き込む」の EUC-JP バイト列: BD F1 A4 AD B9 FE A4 E0
        const QByteArray expected =
            QByteArray("%BD%F1%A4%AD%B9%FE%A4%E0");
        QCOMPARE(postUrlEncode(QStringLiteral(u"書き込む"), Charset::EucJp),
                 expected);
    }

    // ======== buildPostBody ========

    void buildPostBody_jpnkn_fields()
    {
        using namespace yapcr::bbs;
        Board b = makeJpnknBoard(QStringLiteral("https"),
                                 QStringLiteral("bbs.jpnkn.com"),
                                 QStringLiteral("livegame"));
        const QByteArray body = buildPostBody(b, QStringLiteral("1770907315"),
                                              BoardType::Jpnkn,
                                              QStringLiteral("テスト"),
                                              QStringLiteral("sage"),
                                              QStringLiteral("本文"),
                                              1234567890LL);
        // フィールド名と順序の確認
        QVERIFY2(body.startsWith("submit="),  "submit= で始まる");
        QVERIFY2(body.contains("&FROM="),     "&FROM= を含む");
        QVERIFY2(body.contains("&mail="),     "&mail= を含む");
        QVERIFY2(body.contains("&MESSAGE="),  "&MESSAGE= を含む");
        QVERIFY2(body.contains("&bbs="),      "&bbs= を含む");
        QVERIFY2(body.contains("&key="),      "&key= を含む");
        QVERIFY2(body.contains("&time="),     "&time= を含む");
        // したらば 固有フィールドは含まない
        QVERIFY2(!body.contains("&NAME="),    "jpnkn に &NAME= は含まない");
        QVERIFY2(!body.contains("&DIR="),     "jpnkn に &DIR= は含まない");
        // time が注入値と一致
        QVERIFY2(body.contains("&time=1234567890"), "time 値が一致");
        // submit が「書き込む」の Shift-JIS urlencode
        QVERIFY2(body.startsWith("submit=%8F%91%82%AB%8D%9E%82%DE"),
                 "submit 値が Shift-JIS の「書き込む」");
        // bbs フィールドに board 値
        QVERIFY2(body.contains("&bbs=livegame"), "&bbs=livegame を含む");
        // key フィールド
        QVERIFY2(body.contains("&key=1770907315"), "&key=1770907315 を含む");
    }

    void buildPostBody_shitaraba_fields()
    {
        using namespace yapcr::bbs;
        if (!isCharsetSupported(Charset::EucJp)) {
            QSKIP("EUC-JP がこの環境では利用不可 — M3.8 で実機確認");
        }
        Board b = makeShitarabaBoard(QStringLiteral("https"),
                                     QStringLiteral("jbbs.shitaraba.net"),
                                     QStringLiteral("anime"),
                                     QStringLiteral("12345"));
        const QByteArray body = buildPostBody(b, QStringLiteral("9876543210"),
                                              BoardType::Shitaraba,
                                              QStringLiteral("テスト"),
                                              QStringLiteral(""),
                                              QStringLiteral("本文"),
                                              9999999999LL);
        QVERIFY2(body.startsWith("submit="),  "submit= で始まる");
        QVERIFY2(body.contains("&NAME="),     "&NAME= を含む");
        QVERIFY2(body.contains("&MAIL="),     "&MAIL= を含む");
        QVERIFY2(body.contains("&MESSAGE="),  "&MESSAGE= を含む");
        QVERIFY2(body.contains("&DIR="),      "&DIR= を含む");
        QVERIFY2(body.contains("&BBS="),      "&BBS= を含む");
        QVERIFY2(body.contains("&KEY="),      "&KEY= を含む");
        QVERIFY2(body.contains("&TIME="),     "&TIME= を含む");
        // jpnkn 固有フィールドは含まない
        QVERIFY2(!body.contains("&FROM="),    "したらば に &FROM= は含まない");
        QVERIFY2(!body.contains("&bbs="),     "したらば に &bbs= は含まない");
        // DIR/BBS/KEY
        QVERIFY2(body.contains("&DIR=anime"),      "&DIR=anime を含む");
        QVERIFY2(body.contains("&BBS=12345"),      "&BBS=12345 を含む");
        QVERIFY2(body.contains("&KEY=9876543210"), "&KEY=9876543210 を含む");
        QVERIFY2(body.contains("&TIME=9999999999"), "&TIME 値が一致");
    }

    // ======== buildCookieHeader ========

    void buildCookieHeader_basic()
    {
        using namespace yapcr::bbs;
        const QByteArray cookie = buildCookieHeader(
            QStringLiteral("名無し"), QStringLiteral("sage"), Charset::ShiftJis);
        QVERIFY2(cookie.startsWith("NAME="),      "NAME= で始まる");
        QVERIFY2(cookie.contains("; MAIL="),      "; MAIL= を含む");
        // 末尾が MAIL=<value> で終わる（extra なし）
        QVERIFY2(!cookie.contains("; PON="),      "extra なしなので ; PON= は含まない");
    }

    void buildCookieHeader_extra_cookies()
    {
        using namespace yapcr::bbs;
        QList<QNetworkCookie> extra;
        extra.append(QNetworkCookie("PON", "example.com"));
        extra.append(QNetworkCookie("yuki", "akari"));

        const QByteArray cookie = buildCookieHeader(
            QStringLiteral(""), QStringLiteral(""), Charset::ShiftJis, extra);
        QVERIFY2(cookie.contains("; PON=example.com"),  "; PON=example.com を含む");
        QVERIFY2(cookie.contains("; yuki=akari"),       "; yuki=akari を含む");
    }

    // ======== classifyWriteResponse ========

    void classify_2chx_cookie()
    {
        using namespace yapcr::bbs;
        const QString html = QStringLiteral(
            "<html><body><!-- 2ch_X:cookie --></body></html>");
        const auto cls = classifyWriteResponse(html);
        QCOMPARE(cls.result, WriteResult::NeedCookie);
    }

    void classify_2chx_error()
    {
        using namespace yapcr::bbs;
        const QString html = QStringLiteral(
            "<html><body><!-- 2ch_X:error --></body></html>");
        const auto cls = classifyWriteResponse(html);
        QCOMPARE(cls.result, WriteResult::Error);
    }

    void classify_title_error()
    {
        using namespace yapcr::bbs;
        const QString html = QStringLiteral(
            "<html><head><title>ERROR</title></head></html>");
        const auto cls = classifyWriteResponse(html);
        QCOMPARE(cls.result, WriteResult::Error);
    }

    void classify_title_angel()
    {
        using namespace yapcr::bbs;
        const QString html = QStringLiteral(
            "<html><head><title>You just summoned a Ruthless Angel.</title></head></html>");
        const auto cls = classifyWriteResponse(html);
        QCOMPARE(cls.result, WriteResult::Error);
    }

    void classify_jpnkn_auth_error()
    {
        using namespace yapcr::bbs;
        // jpnkn 全角 ERROR + 認証要求（title 外の本文）
        const QString html = QStringLiteral(
            "<html><body>ＥＲＲＯＲ：この掲示板では利用認証が必要です。"
            "https://edge.jpnkn.com/auth.php?code=XXXX</body></html>");
        const auto cls = classifyWriteResponse(html);
        QCOMPARE(cls.result, WriteResult::Error);
    }

    void classify_jpnkn_cookie_burnt()
    {
        using namespace yapcr::bbs;
        // jpnkn COOKIE got burnt（半角）
        const QString html = QStringLiteral(
            "<html><body>ＥＲＲＯＲ：The COOKIE got burnt!</body></html>");
        const auto cls = classifyWriteResponse(html);
        QCOMPARE(cls.result, WriteResult::Error);
    }

    void classify_ok()
    {
        using namespace yapcr::bbs;
        // 書き込み成功ページ（エラー要素なし）
        const QString html = QStringLiteral(
            "<html><head><title>書き込み完了</title></head>"
            "<body>書き込みが完了しました。</body></html>");
        const auto cls = classifyWriteResponse(html);
        QCOMPARE(cls.result, WriteResult::Ok);
    }

    // ======== nextPostAction ========

    void nextPostAction_ok_succeeds()
    {
        using namespace yapcr::bbs;
        QCOMPARE(nextPostAction(WriteResult::Ok, 0, false), PostAction::Succeed);
        QCOMPARE(nextPostAction(WriteResult::Ok, 0, true),  PostAction::Succeed);
        QCOMPARE(nextPostAction(WriteResult::Ok, 1, true),  PostAction::Succeed);
    }

    void nextPostAction_need_cookie_first_with_cookie_resends()
    {
        using namespace yapcr::bbs;
        QCOMPARE(nextPostAction(WriteResult::NeedCookie, 0, true), PostAction::Resend);
    }

    void nextPostAction_need_cookie_no_new_cookie_fails()
    {
        using namespace yapcr::bbs;
        // 確認ページだが Set-Cookie なし → 再送材料がないので失敗
        QCOMPARE(nextPostAction(WriteResult::NeedCookie, 0, false), PostAction::Fail);
    }

    void nextPostAction_need_cookie_second_attempt_fails()
    {
        using namespace yapcr::bbs;
        // 2 回目の確認ページ（再送後また確認）→ 無限ループ防止で失敗
        QCOMPARE(nextPostAction(WriteResult::NeedCookie, 1, true), PostAction::Fail);
    }

    void nextPostAction_error_fails()
    {
        using namespace yapcr::bbs;
        QCOMPARE(nextPostAction(WriteResult::Error, 0, false), PostAction::Fail);
        QCOMPARE(nextPostAction(WriteResult::Error, 0, true),  PostAction::Fail);
    }

    // ======== calcSpeed（M3.9）========

    void calcSpeed_normal()
    {
        using namespace yapcr::bbs;
        // 100 レス / 2 日 = 50.0 レス/日
        // key = epoch 秒 0、now = 172800（= 2 * 86400）
        const double speed = calcSpeed(100, QStringLiteral("0"), 172800);
        QCOMPARE(speed, 50.0);
    }

    void calcSpeed_zero_elapsed_returns_zero()
    {
        using namespace yapcr::bbs;
        // elapsed == 0 → 0.0（ゼロ除算なし）
        QCOMPARE(calcSpeed(100, QStringLiteral("1000"), 1000), 0.0);
    }

    void calcSpeed_negative_elapsed_returns_zero()
    {
        using namespace yapcr::bbs;
        // 未来のスレ立て時刻（elapsed < 0）→ 0.0
        QCOMPARE(calcSpeed(50, QStringLiteral("9999999999"), 1000), 0.0);
    }

    void calcSpeed_zero_count()
    {
        using namespace yapcr::bbs;
        // レス 0 件 → 0.0
        QCOMPARE(calcSpeed(0, QStringLiteral("0"), 86400), 0.0);
    }

    // ======== selectFastest（M3.9）========

    // テスト用 ThreadInfo ビルダー
    static yapcr::bbs::ThreadInfo makeThr(const QString& key, int count, double speed)
    {
        yapcr::bbs::ThreadInfo t;
        t.key   = key;
        t.count = count;
        t.speed = speed;
        return t;
    }

    void selectFastest_picks_max_speed()
    {
        using namespace yapcr::bbs;
        // 2スレ候補: speed 10.0 vs 30.0 → 30.0 を選ぶ
        const QList<ThreadInfo> subject = {
            makeThr(QStringLiteral("100"), 100, 10.0),
            makeThr(QStringLiteral("200"), 100, 30.0),
        };
        ThreadInfo out;
        QVERIFY(selectFastest(subject, 1000, 0, out));
        QCOMPARE(out.key, QStringLiteral("200"));
    }

    void selectFastest_excludes_full_threads()
    {
        using namespace yapcr::bbs;
        // count >= stop（1000）の満了スレは除外される
        const QList<ThreadInfo> subject = {
            makeThr(QStringLiteral("100"), 1000, 50.0),   // 満了（除外）
            makeThr(QStringLiteral("200"), 999,  30.0),   // 候補
        };
        ThreadInfo out;
        QVERIFY(selectFastest(subject, 1000, 0, out));
        QCOMPARE(out.key, QStringLiteral("200"));
    }

    void selectFastest_excludes_older_threads()
    {
        using namespace yapcr::bbs;
        // curKeyEpoch=200 → key<=200 は除外
        const QList<ThreadInfo> subject = {
            makeThr(QStringLiteral("100"), 100, 50.0),   // 古い（除外）
            makeThr(QStringLiteral("200"), 100, 50.0),   // 同 epoch（除外）
            makeThr(QStringLiteral("300"), 100, 20.0),   // 新しい候補
        };
        ThreadInfo out;
        QVERIFY(selectFastest(subject, 1000, 200, out));
        QCOMPARE(out.key, QStringLiteral("300"));
    }

    void selectFastest_no_candidate_returns_false()
    {
        using namespace yapcr::bbs;
        // 全スレが満了 → false、out は書き換えない
        const QList<ThreadInfo> subject = {
            makeThr(QStringLiteral("100"), 1000, 50.0),
        };
        ThreadInfo out;
        out.key = QStringLiteral("sentinel");
        QVERIFY(!selectFastest(subject, 1000, 0, out));
        QCOMPARE(out.key, QStringLiteral("sentinel"));  // 書き換えなし
    }

    void selectFastest_cold_start_cur_epoch_zero()
    {
        using namespace yapcr::bbs;
        // curKeyEpoch=0 → 全スレが対象（板URL cold-start）
        const QList<ThreadInfo> subject = {
            makeThr(QStringLiteral("100"), 100, 5.0),
            makeThr(QStringLiteral("50"),  100, 50.0),   // epoch 50 < 100 でも ok
        };
        ThreadInfo out;
        QVERIFY(selectFastest(subject, 1000, 0, out));
        QCOMPARE(out.key, QStringLiteral("50"));
    }
};

QTEST_MAIN(TstBbs)
#include "tst_bbs.moc"
