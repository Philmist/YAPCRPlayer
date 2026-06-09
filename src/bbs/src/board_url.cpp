#include "bbs/board_url.h"

#include <QRegularExpression>

namespace yapcr::bbs {

namespace {

// スキーム接頭辞（"https", "ttp", "tp" 等）から正規化された http/https を返す。
// 移植元: makeScheme (BaseBBS.cpp:66-78)。
// 原典の boost::xpressive を QRegularExpression に置換。
static QString makeScheme(const QString& prefix)
{
    static const QRegularExpression rx(
        QStringLiteral(R"((?:http|ttp|tp)(s)?$)"));
    const QRegularExpressionMatch m = rx.match(prefix);
    if (m.hasMatch()) {
        // captured(1) は "s" または空文字（非マッチ時）
        return QStringLiteral("http") + m.captured(1);
    }
    return QStringLiteral("http");
}

}  // namespace

// M5: config 化（したらば ホスト文字列を外部設定から読む）
BoardType detectBoardType(const QString& host)
{
    return host == QStringLiteral("jbbs.shitaraba.net")
        ? BoardType::Shitaraba
        : BoardType::Jpnkn;
}

BoardLocation parseContactUrl(const QString& url)
{
    BoardLocation loc;

    // ── ホスト抽出 ─────────────────────────────────
    static const QRegularExpression rxHost(
        QStringLiteral(R"(.*://([^/]+))"));
    const QRegularExpressionMatch mHost = rxHost.match(url);
    if (!mHost.hasMatch()) { return loc; }

    const QString host = mHost.captured(1);
    loc.type = detectBoardType(host);

    // ── したらば ──────────────────────────────────
    // 移植元: ShitarabaBBS::ShitarabaBBS (BaseBBS.cpp:764-819)
    // フィールドマッピング: scheme, host(=base), board, number, key
    if (loc.type == BoardType::Shitaraba) {
        // 原典は `url.find(L"bbs\read.cgi")` だが '\r' になるバグがあるため
        // 正しく "bbs/read.cgi" で判定する。
        const bool hasReadCgi = url.contains(QStringLiteral("bbs/read.cgi"));
        QRegularExpression rx;
        if (hasReadCgi) {
            // thread 直リンク: scheme://host/bbs/read.cgi/board/number/key/
            rx.setPattern(
                QStringLiteral(R"((.*)://([^/]+)/bbs/read\.cgi/([^/]+)/([^/]+)/([^/]+))"));
        } else {
            // 板トップ: scheme://host/board/number/
            rx.setPattern(
                QStringLiteral(R"((.*)://([^/]+)/([^/]+)/([^/]+))"));
        }
        const QRegularExpressionMatch m = rx.match(url);
        if (!m.hasMatch()) { return loc; }

        loc.board.scheme = makeScheme(m.captured(1));
        loc.board.host   = m.captured(2);
        loc.board.base   = m.captured(2);  // したらば: base = host のみ
        loc.board.board  = m.captured(3);
        loc.board.number = m.captured(4);
        loc.board.code   = Charset::EucJp;
        if (hasReadCgi) {
            loc.thread.key = m.captured(5);
        }
        loc.valid = true;

    // ── jpnkn (2ch 系) ───────────────────────────
    // 移植元: BaseBBS::BaseBBS (BaseBBS.cpp:87-140)
    // フィールドマッピング: scheme, base, host, board, key（number は常に空）
    } else {
        const bool hasReadCgi = url.contains(QStringLiteral("test/read.cgi"));
        QRegularExpression rx;
        if (hasReadCgi) {
            // thread 直リンク: scheme://base/test/read.cgi/board/key/
            rx.setPattern(
                QStringLiteral(R"((.*)://(([^/]+).*)/test/read\.cgi/([^/]+)/([^/]+))"));
        } else {
            // 板トップ: scheme://base/board/
            rx.setPattern(
                QStringLiteral(R"((.*)://(([^/]+).*)/([^/]+))"));
        }
        const QRegularExpressionMatch m = rx.match(url);
        if (!m.hasMatch()) { return loc; }

        loc.board.scheme = makeScheme(m.captured(1));
        loc.board.base   = m.captured(2);  // jpnkn: base = host + パス接頭辞込み
        loc.board.host   = m.captured(3);
        loc.board.board  = m.captured(4);
        // number は未設定のまま（jpnkn は board/number 分離なし）
        loc.board.code   = Charset::ShiftJis;
        if (hasReadCgi) {
            loc.thread.key = m.captured(5);
        }
        loc.valid = true;
    }

    // URL を充填
    loc.board.url  = boardUrl(loc.board, loc.type);
    loc.thread.url = threadUrl(loc.board, loc.thread.key, loc.type);

    return loc;
}

// ---------- URL ビルダー ----------

// 移植元: BaseBBS::board (BaseBBS.cpp:689)
// したらば: ShitarabaBBS::board (BaseBBS.cpp:893)
QString boardUrl(const Board& b, BoardType type)
{
    if (type == BoardType::Shitaraba) {
        return b.scheme + QStringLiteral("://")
            + b.base   + QStringLiteral("/")
            + b.board  + QStringLiteral("/")
            + b.number + QStringLiteral("/");
    }
    return b.scheme + QStringLiteral("://")
        + b.base  + QStringLiteral("/")
        + b.board + QStringLiteral("/");
}

// 移植元: BaseBBS::thread (BaseBBS.cpp:696)
// したらば: ShitarabaBBS::thread (BaseBBS.cpp:901)
QString threadUrl(const Board& b, const QString& key, BoardType type)
{
    if (type == BoardType::Shitaraba) {
        return b.scheme + QStringLiteral("://")
            + b.base   + QStringLiteral("/bbs/read.cgi/")
            + b.board  + QStringLiteral("/")
            + b.number + QStringLiteral("/")
            + key      + QStringLiteral("/");
    }
    return b.scheme + QStringLiteral("://")
        + b.base  + QStringLiteral("/test/read.cgi/")
        + b.board + QStringLiteral("/")
        + key     + QStringLiteral("/");
}

// 移植元: BaseBBS::setting (BaseBBS.cpp:704)
// したらば: ShitarabaBBS::setting (BaseBBS.cpp:910)
QString settingUrl(const Board& b, BoardType type)
{
    if (type == BoardType::Shitaraba) {
        return b.scheme + QStringLiteral("://")
            + b.base   + QStringLiteral("/bbs/api/setting.cgi/")
            + b.board  + QStringLiteral("/")
            + b.number + QStringLiteral("/");
    }
    return b.scheme + QStringLiteral("://")
        + b.base  + QStringLiteral("/")
        + b.board + QStringLiteral("/SETTING.TXT");
}

// 移植元: BaseBBS::subject (BaseBBS.cpp:711)
// したらば: ShitarabaBBS::subject (BaseBBS.cpp:918)
QString subjectUrl(const Board& b, BoardType type)
{
    if (type == BoardType::Shitaraba) {
        return b.scheme + QStringLiteral("://")
            + b.base   + QStringLiteral("/")
            + b.board  + QStringLiteral("/")
            + b.number + QStringLiteral("/subject.txt");
    }
    return b.scheme + QStringLiteral("://")
        + b.base  + QStringLiteral("/")
        + b.board + QStringLiteral("/subject.txt");
}

// 移植元: BaseBBS::dat (BaseBBS.cpp:718)
// したらば: ShitarabaBBS::dat (BaseBBS.cpp:926)
QString datUrl(const Board& b, const QString& key, BoardType type)
{
    if (type == BoardType::Shitaraba) {
        return b.scheme + QStringLiteral("://")
            + b.base   + QStringLiteral("/bbs/rawmode.cgi/")
            + b.board  + QStringLiteral("/")
            + b.number + QStringLiteral("/")
            + key      + QStringLiteral("/");
    }
    return b.scheme + QStringLiteral("://")
        + b.base  + QStringLiteral("/")
        + b.board + QStringLiteral("/dat/")
        + key     + QStringLiteral(".dat");
}

// 移植元: BaseBBS::write (BaseBBS.cpp:726)
// したらば: ShitarabaBBS::write (BaseBBS.cpp:935)
QString writeUrl(const Board& b, BoardType type)
{
    if (type == BoardType::Shitaraba) {
        return b.scheme + QStringLiteral("://")
            + b.base + QStringLiteral("/bbs/write.cgi");
    }
    return b.scheme + QStringLiteral("://")
        + b.base + QStringLiteral("/test/bbs.cgi");
}

}  // namespace yapcr::bbs
