#pragma once

#include "bbs/models.h"

#include <QList>
#include <QString>

namespace yapcr::bbs {

// 復号済み subject.txt をパースして ThreadInfo 一覧を返す。
// type で行書式を切替:
//   Jpnkn:     key.dat<>title (count)   移植元: BaseBBS::parser(ThreadInfo&)@610
//   Shitaraba: key.cgi,title(count)     移植元: ShitarabaBBS::parser(ThreadInfo&)@845
//
// number は空・speed は 0 のまま（連番付与/dedup/速度算出は M3.4 の責務）。
QList<ThreadInfo> parseSubject(const QString& text, BoardType type);

}  // namespace yapcr::bbs
