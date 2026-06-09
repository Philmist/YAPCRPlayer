#include "bbs/dat.h"

#include <QRegularExpression>
#include <QStringList>

namespace yapcr::bbs {

namespace {

// ── removeATag ──────────────────────────────────────────────────────────────
// <a ...> と </a> のタグ部分のみを除去し、テキスト内容と <br> 等は残す。
// 移植元: HTMLEscape.h:99 TagRemover("a")
//   compile: as_xpr('<') >> !(-*_s >> '/') >> -*_s >> icase("a") >> ~before(alpha) >> -*~'>' >> '>'
// NOTE: 専ブラ表示オミット設計のため message を <a> に書き換える操作は行わない。
static QString removeATag(const QString& html)
{
    // <  省略可能(空白*/? ) 空白* a  英字が続かない  任意の非> 文字列 >
    static const QRegularExpression rx(
        QStringLiteral(R"(<\s*/?\s*a(?![A-Za-z])[^>]*>)"),
        QRegularExpression::CaseInsensitiveOption);
    QString result = html;
    result.remove(rx);
    return result;
}

// ── datetimeid (jpnkn 専用) ────────────────────────────────────────────────
// datetime 列から res.datetime と res.id を抽出して設定する。
// 移植元: BaseBBS::datetimeid()@444
//   rxDatetime: ([0-9]+/[0-9]+/[0-9]+.*?)\s*([0-9]+:[0-9]+:[0-9]+\S*)
//     → "日付(曜日) 時刻" を再構成（(曜日) を含む）
//   rxID: ID:(\S*)
// マッチしない場合は datetime 列をそのまま res.datetime に設定し false を返す。
static bool datetimeid(ResInfo& res, const QString& datetime)
{
    static const QRegularExpression rxDatetime(
        QStringLiteral(R"(([0-9]+/[0-9]+/[0-9]+.*?)\s*([0-9]+:[0-9]+:[0-9]+\S*))"));
    static const QRegularExpression rxID(
        QStringLiteral(R"(ID:(\S*))"));

    const QRegularExpressionMatch mDate = rxDatetime.match(datetime);
    if (!mDate.hasMatch()) {
        return false;
    }
    // 日付グループ（曜日を含む）+ 空白 + 時刻グループ
    res.datetime = mDate.captured(1) + QLatin1Char(' ') + mDate.captured(2);

    const QRegularExpressionMatch mID = rxID.match(datetime);
    if (mID.hasMatch()) {
        res.id = mID.captured(1);
    } else {
        res.id.clear();
    }
    return true;
}

// ── jpnkn 1行パース ──────────────────────────────────────────────────────────
// 書式: name<>mail<>datetime<>message<>title
// 移植元: BaseBBS::parser(ResInfo&)@655
// number は空のまま（M3.4 の BbsSession が読込順を付与する）
static bool parseJpnknLine(ResInfo& res, const QString& line)
{
    // "<>" で最大 5 列に分割（5 列目以降は無視）
    const QStringList cols = line.split(QStringLiteral("<>"));
    if (cols.size() < 3) { return false; }

    res.name = cols.at(0);
    res.mail = cols.at(1);

    const QString& dt = cols.at(2);
    if (!datetimeid(res, dt)) {
        res.datetime = dt;
    }

    if (cols.size() >= 4) {
        res.message = removeATag(cols.at(3));
    }
    if (cols.size() >= 5) {
        res.title = cols.at(4);
    }
    return true;
}

// ── したらば 1行パース ────────────────────────────────────────────────────────
// 書式: number<>name<>mail<>datetime<>message<>title<>id
// 移植元: ShitarabaBBS::parser(ResInfo&)@865
static bool parseShitarabaLine(ResInfo& res, const QString& line)
{
    const QStringList cols = line.split(QStringLiteral("<>"));
    if (cols.size() < 5) { return false; }

    res.number   = cols.at(0);
    res.name     = cols.at(1);
    res.mail     = cols.at(2);
    res.datetime = cols.at(3);  // したらば は datetime をそのまま格納

    res.message = removeATag(cols.at(4));

    if (cols.size() >= 6) {
        res.title = cols.at(5);
    }
    if (cols.size() >= 7) {
        res.id = cols.at(6);
    }
    return true;
}

}  // namespace

QList<ResInfo> parseDat(const QString& text, BoardType type)
{
    QList<ResInfo> result;

    const QStringList lines = text.split(u'\n');
    for (const QString& rawLine : lines) {
        // 行末 \r を除去
        const QString line = rawLine.endsWith(u'\r')
            ? rawLine.left(rawLine.size() - 1)
            : rawLine;

        if (line.isEmpty()) { continue; }

        ResInfo res;
        const bool ok = (type == BoardType::Shitaraba)
            ? parseShitarabaLine(res, line)
            : parseJpnknLine    (res, line);

        if (ok) {
            result.append(res);
        }
    }

    return result;
}

}  // namespace yapcr::bbs
