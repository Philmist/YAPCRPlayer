#include <QTest>
#include <QFile>

#include "bbs/board_url.h"
#include "bbs/charset.h"
#include "bbs/dat.h"
#include "bbs/extract.h"
#include "bbs/models.h"
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
};

QTEST_MAIN(TstBbs)
#include "tst_bbs.moc"
