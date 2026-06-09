#include "bbs/subject.h"

#include <QRegularExpression>
#include <QStringList>

namespace yapcr::bbs {

namespace {

// ── jpnkn 1行パース ──────────────────────────────────────
// 書式: key.dat<>title (count)
// 移植元: BaseBBS::parser(ThreadInfo&)@610
//   case0: key 列を unanchored regex_search(\d+) で取得
//   case1: title 列を end-anchored (eos 付き) で title/count 分割
static bool parseJpnknLine(ThreadInfo& thr, const QString& line)
{
    // "<>" で 2列に分割
    const int sep = line.indexOf(QStringLiteral("<>"));
    if (sep < 0) { return false; }

    const QString keyCol   = line.left(sep);
    const QString titleCol = line.mid(sep + 2);

    // key: unanchored（移植元 case0 は regex_search）— 先頭の連続数字列を key とする
    static const QRegularExpression rxKey(QStringLiteral(R"((\d+))"));
    const QRegularExpressionMatch mKey = rxKey.match(keyCol);
    if (!mKey.hasMatch()) { return false; }
    thr.key = mKey.captured(1);

    // title / count: end-anchored（移植元 case1 の *_s >> '(' >> +digit >> ')' >> *_s >> eos）
    // non-greedy title で末尾 (n) を count に取る（題名中の括弧に耐える）
    static const QRegularExpression rxTitle(QStringLiteral(R"(^(.*?)\s*\((\d+)\)\s*$)"));
    const QRegularExpressionMatch mTitle = rxTitle.match(titleCol);
    if (!mTitle.hasMatch()) { return false; }

    thr.title = mTitle.captured(1);
    bool ok = false;
    thr.count = mTitle.captured(2).toInt(&ok);
    return ok;
}

// ── したらば 1行パース ────────────────────────────────────
// 書式: key.cgi,title(count)
// 移植元: ShitarabaBBS::parser(ThreadInfo&)@845（regex_match = 完全 anchor）
static bool parseShitarabaLine(ThreadInfo& thr, const QString& line)
{
    // QRegularExpression::anchoredPattern で移植元の regex_match を再現する。
    // (.*)  はグリーディなので末尾の最後の (digits) が count になる。
    static const QRegularExpression rxLine(
        QRegularExpression::anchoredPattern(
            QStringLiteral(R"((\d+)\.cgi,(.*)\((\d+)\))")));
    const QRegularExpressionMatch m = rxLine.match(line);
    if (!m.hasMatch()) { return false; }

    thr.key   = m.captured(1);
    thr.title = m.captured(2);
    bool ok = false;
    thr.count = m.captured(3).toInt(&ok);
    return ok;
}

}  // namespace

QList<ThreadInfo> parseSubject(const QString& text, BoardType type)
{
    QList<ThreadInfo> result;

    const QStringList lines = text.split(u'\n');
    for (const QString& rawLine : lines) {
        // 行末 \r を除去
        const QString line = rawLine.endsWith(u'\r')
            ? rawLine.left(rawLine.size() - 1)
            : rawLine;

        if (line.isEmpty()) { continue; }

        ThreadInfo thr;
        const bool ok = (type == BoardType::Shitaraba)
            ? parseShitarabaLine(thr, line)
            : parseJpnknLine    (thr, line);

        if (ok) {
            result.append(thr);
        }
    }

    return result;
}

}  // namespace yapcr::bbs
