#include "bbs/extract.h"

#include <QRegularExpression>
#include <QRegularExpressionMatchIterator>
#include <QSet>
#include <QString>

namespace yapcr::bbs {

namespace {

// ── 全角→半角 数字変換 ─────────────────────────────────────────────────────
// 移植元: BBSRegex::convert(wchar_t) / BBSRegex.cpp:26-38
// 全角 ０-９（U+FF10〜U+FF19）を ASCII 0-9 に変換。それ以外はそのまま返す。
static QChar fullWidthToHalf(QChar c)
{
    const ushort v = c.unicode();
    if (v >= 0xFF10 && v <= 0xFF19) {
        return QChar(u'0' + (v - 0xFF10));
    }
    return c;
}

// 全角数字を含む文字列を ASCII 数字文字列に変換する。
// 移植元: BBSRegex::convert(std::wstring) / BBSRegex.cpp:41-49
static QString fullWidthToHalfStr(const QString& s)
{
    QString result;
    result.reserve(s.size());
    for (const QChar c : s) {
        result += fullWidthToHalf(c);
    }
    return result;
}

// 数字文字列（半角変換済み）を int に変換して Range を構築する。
// 移植元: BBSRegex::convert(text, Range&) / BBSRegex.cpp:6-24
//   hyphen 区切りで分割 → 数値集合 → min/max を first/last に
static Range buildRange(const QString& pairStr, const QString& hyphenPat)
{
    static const QRegularExpression rxHyphen(hyphenPat);
    const QStringList parts = pairStr.split(rxHyphen, Qt::SkipEmptyParts);

    QSet<int> vals;
    for (const QString& p : parts) {
        const QString h = fullWidthToHalfStr(p.trimmed());
        if (!h.isEmpty()) {
            bool ok = false;
            const int n = h.toInt(&ok);
            if (ok) { vals.insert(n); }
        }
    }

    if (vals.isEmpty()) {
        return Range{};
    }

    const int mn = *std::min_element(vals.cbegin(), vals.cend());
    const int mx = *std::max_element(vals.cbegin(), vals.cend());
    return Range{mn, mx};
}

// アンカー行全体（">>1-3,5" 相当の既マッチ文字列）から Range 一覧を生成する。
// 移植元: BBSReplace::anchor / BBSRegex.cpp:150-179
static QList<Range> parseAnchorStr(const QString& anchorMatch)
{
    // pair/groupe 分解:
    //   gt2    = (?:&gt;|＞){1,2}
    //   number = [0-9０-９]+
    //   hyphen = (?:-|－)
    //   comma  = (?:,|，)
    //   pair   = number (hyphen number)*
    //   groupe = pair (comma pair)*

    // カンマ（半角・全角）で pair を分割
    static const QRegularExpression rxComma(QStringLiteral(R"((?:,|，))"));
    // ハイフン（半角・全角）
    const QString hyphenPat = QStringLiteral(R"((?:-|－))");

    QList<Range> result;

    // アンカー頭（>> または ＞＞ や &gt;&gt;）を除いた部分を取得
    // anchorMatch 例: "&gt;&gt;1-3,5"  "＞＞１２３"
    static const QRegularExpression rxGt(QStringLiteral(R"(^(?:(?:&gt;|＞){1,2}))"));
    const QString body = QString(anchorMatch).remove(rxGt);  // gt 先頭を除去

    const QStringList pairs = body.split(rxComma, Qt::SkipEmptyParts);
    for (const QString& pairStr : pairs) {
        const Range r = buildRange(pairStr, hyphenPat);
        if (r.first != 0 || r.last != 0) {
            result.append(r);
        } else {
            // 単数アンカー（hyphen なし）
            const QString h = fullWidthToHalfStr(pairStr.trimmed());
            bool ok = false;
            const int n = h.toInt(&ok);
            if (ok && n != 0) {
                result.append(Range{n, n});
            }
        }
    }
    return result;
}

// アンカーパターン（gt2 groupe）。extractAnchors / extractAnchorSpans で共用する。
//   gt2    = (?:&gt;|＞){1,2}
//   number = [0-9０-９]+
//   hyphen = (?:-|－)  comma = (?:,|，)
//   pair   = number(hyphen number)*   groupe = pair(comma pair)*
// NOTE: dat 内は HTML エスケープ済みのため &gt; に当てる。ASCII > には当てない。
static const QRegularExpression& anchorRegex()
{
    static const QRegularExpression rx(
        QStringLiteral(
            R"((?:&gt;|＞){1,2})"          // gt2
            R"([0-9０-９]+)"                // number (pair 先頭)
            R"((?:(?:-|－)[0-9０-９]+)*)"   // (hyphen number)*
            R"((?:(?:,|，)[0-9０-９]+(?:(?:-|－)[0-9０-９]+)*)*)" // (comma pair)*
        ));
    return rx;
}

// parseAnchorStr の複数 Range を min(first)..max(last) の包含 Range に畳む。
static Range boundingRange(const QList<Range>& ranges)
{
    if (ranges.isEmpty()) { return Range{}; }
    int mn = ranges.first().first;
    int mx = ranges.first().last;
    for (const Range& r : ranges) {
        mn = qMin(mn, qMin(r.first, r.last));
        mx = qMax(mx, qMax(r.first, r.last));
    }
    return Range{mn, mx};
}

}  // namespace

// ── extractUrls ──────────────────────────────────────────────────────────────
// 移植元: BBSReplace::url / BBSReplace::scheme
// scheme:    [A-Za-z0-9_]*:
// body:      //[A-Za-z0-9_#$@&!?~:;.,=%/*+\-]+
// 正規化:    http|ttp|tp + (s?) → http + (s?)
QList<QString> extractUrls(const QString& message)
{
    // 末尾の . や , は URL に含めない（移植元 body の末尾にならう厳密定義ではないが実用的）
    static const QRegularExpression rx(
        QStringLiteral(R"(([A-Za-z0-9_]*):(//[A-Za-z0-9_#$@&!?~:;.,=%/*+\-]+))"));
    // スキーム正規化用
    static const QRegularExpression rxScheme(
        QStringLiteral(R"(^(?:http|ttp|tp)(s?)$)"));

    QList<QString> urls;
    QRegularExpressionMatchIterator it = rx.globalMatch(message);
    while (it.hasNext()) {
        const QRegularExpressionMatch m = it.next();
        const QString scheme = m.captured(1);
        const QRegularExpressionMatch ms = rxScheme.match(scheme);
        if (ms.hasMatch()) {
            // ttp:// → http://、tp:// → http://、https:// はそのまま
            const QString normalized = QStringLiteral("http") + ms.captured(1) + QStringLiteral(":") + m.captured(2);
            urls.append(normalized);
        }
    }
    return urls;
}

// ── extractAnchors ────────────────────────────────────────────────────────────
// 移植元: BBSReplace::anchor / BBSRegex::groupe / BBSRegex.cpp:150-179
// アンカーパターン: gt2 groupe
//   gt2    = (?:&gt;|＞){1,2}
//   number = [0-9０-９]+
//   hyphen = (?:-|－)
//   comma  = (?:,|，)
//   pair   = number(hyphen number)*
//   groupe = pair(comma pair)*
// NOTE: dat 内は HTML エスケープ済みのため &gt; に当てる。ASCII > には当てない。
QList<Range> extractAnchors(const QString& message)
{
    QList<Range> result;
    QRegularExpressionMatchIterator it = anchorRegex().globalMatch(message);
    while (it.hasNext()) {
        const QRegularExpressionMatch m = it.next();
        const QList<Range> ranges = parseAnchorStr(m.captured(0));
        result.append(ranges);
    }
    return result;
}

// ── extractAnchorSpans ─────────────────────────────────────────────────────────
// extractAnchors と同一の正規表現でマッチし、位置（start/length）と
// 包含 Range を返す。着色用に message 内のアンカー区間を直接包むために使う。
QList<AnchorSpan> extractAnchorSpans(const QString& message)
{
    QList<AnchorSpan> result;
    QRegularExpressionMatchIterator it = anchorRegex().globalMatch(message);
    while (it.hasNext()) {
        const QRegularExpressionMatch m = it.next();
        const Range r = boundingRange(parseAnchorStr(m.captured(0)));
        if (r.first == 0 && r.last == 0) { continue; }  // 数値化できないものは除外
        result.append(AnchorSpan{
            static_cast<int>(m.capturedStart(0)),
            static_cast<int>(m.capturedLength(0)),
            r});
    }
    return result;
}

// ── extractId ────────────────────────────────────────────────────────────────
// 移植元: BBSReplace::id / BBSRegex::head / BBSRegex::serial
// 書式: "ID:" + 8文字以上の [!-~] + 省略可能な終端 [OQo0]
// NOTE: 移植元 serial() は repeat<8>(set[range('!','~')]) >> !(set = 'O','Q','o','0')
//   → 8文字固定 + 省略可能な後置 {O,Q,o,0}
QString extractId(const QString& text)
{
    static const QRegularExpression rx(
        QStringLiteral(R"(ID:([\x21-\x7e]{8}[OQo0]?))"));
    const QRegularExpressionMatch m = rx.match(text);
    if (m.hasMatch()) {
        return m.captured(1);
    }
    return {};
}

}  // namespace yapcr::bbs
