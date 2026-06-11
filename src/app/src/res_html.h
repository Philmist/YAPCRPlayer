#pragma once

#include "bbs/models.h"

#include <QString>

namespace yapcr::app::reshtml {

// 簡易レス表示の配色パレット（ライト背景前提）。後から調整可。
// 一覧ペイン（白 viewport）とポップアップ（白系半透明）の双方で使う。
inline const QString kNumberColor   = QStringLiteral("#556688");  // レス番: 紺寄りグレー
inline const QString kNameSageColor = QStringLiteral("#1b8a1b");  // 名前(sage/メール空): 緑
inline const QString kNameAgeColor  = QStringLiteral("#cc2222");  // 名前(age): 赤
inline const QString kMailColor     = QStringLiteral("#1a5fb4");  // メール欄: 青
inline const QString kTimeColor     = QStringLiteral("#777777");  // 日時・ID: 中間グレー
inline const QString kAnchorColor   = QStringLiteral("#1a5fb4");  // アンカー: 青

// ResInfo 1 件分を 1 レス分の HTML 断片に整形する（区切り <hr> は呼び出し側で付与）。
//   ヘッダ行: 「番号 名前 [メール] 日時 ID」を色分けして1行に並べる。
//   本文行  : <br> を残して他タグ除去、アンカー（>>N / >>1-30）を着色。
// withAnchorLinks=true:
//   レス番を <a href="ref:N">、本文アンカーを <a href="range:first-last"> として埋め込む
//   （一覧ペインの anchorAt() ホバー判定用）。
// withAnchorLinks=false:
//   ナビゲーション不要のため <span> で着色のみ（ポップアップ用）。
QString resToHtml(const yapcr::bbs::ResInfo& r, bool withAnchorLinks);

}  // namespace yapcr::app::reshtml
