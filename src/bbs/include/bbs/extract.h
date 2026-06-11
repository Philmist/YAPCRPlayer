#pragma once

#include "bbs/models.h"

#include <QList>
#include <QString>

namespace yapcr::bbs {

// ── 抽出プリミティブ（read-only 純関数）────────────────────────────────────
// 移植元: BBSRegex.h/BBSRegex.cpp の BBSReplace・BBSRegex を QRegularExpression に移植。
//
// ★ 重要: これらの関数は message を書き換えない（read-only）。
//   移植元 BBSReplace::operator() は抽出と <a href=...> への HTML 置換が一体だが、
//   専ブラ表示オミット設計のため置換は移植しない（M3.4 実装者向け注記）。
//   また dat 内アンカーは HTML エスケープ済みなので &gt; / 全角 ＞ に当てる
//   （ASCII > には当てない）。

// URL を抽出する。スキーム正規化（ttp://, tp:// → http://）を行う。
// 移植元: BBSReplace::url / BBSReplace::scheme
QList<QString> extractUrls(const QString& message);

// アンカー（>>n / n-m / n,m 群）を Range 一覧として抽出する。
// 全角数字・全角 ＞・全角 － ・全角 ，を受ける（横断決定 4）。
// 移植元: BBSReplace::anchor / BBSRegex::convert(text, Range&) / BBSRegex::convert(wchar_t)
QList<Range> extractAnchors(const QString& message);

// アンカーを「マッチ位置付き」で抽出する。着色用に message 内の区間を直接包むために使う。
//   start/length は message 内の文字位置（QString インデックス）。
//   range はそのアンカー1個分の包含範囲（複数 pair の場合は min(first)..max(last)）。
// extractAnchors と同一の正規表現を共用する。
struct AnchorSpan {
    int   start{0};
    int   length{0};
    Range range;
};
QList<AnchorSpan> extractAnchorSpans(const QString& message);

// ID を抽出する。"ID:XXXXXXXX" 形式。見つからない場合は空文字列を返す。
// 移植元: BBSReplace::id / BBSRegex::head / BBSRegex::serial
QString extractId(const QString& text);

}  // namespace yapcr::bbs
