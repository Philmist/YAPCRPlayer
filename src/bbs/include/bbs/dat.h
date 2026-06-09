#pragma once

#include "bbs/models.h"

#include <QList>
#include <QString>

namespace yapcr::bbs {

// 復号済み dat テキストをパースして ResInfo 一覧を返す。
// type で行書式を切り替える:
//   Jpnkn:     name<>mail<>datetime<>message<>title
//              移植元: BaseBBS::parser(ResInfo&)@655 / datetimeid()@444
//   Shitaraba: number<>name<>mail<>datetime<>message<>title<>id
//              移植元: ShitarabaBBS::parser(ResInfo&)@865
//
// NOTE: M3.4 に委譲するフィールド
//   - number (jpnkn): 読込順付与 + 削除穴埋めは M3.4 の BbsSession が担当
//   - link / ref / count / total / identifier / split: M3.4 の prepareDat 相当処理が担当
// NOTE: message は linkify されていない素のテキスト (専ブラ表示オミット設計)。
//   <a>...</a> のタグ部分のみ除去し、<br> 等の他タグは残す。
QList<ResInfo> parseDat(const QString& text, BoardType type);

}  // namespace yapcr::bbs
