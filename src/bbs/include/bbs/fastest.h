#pragma once

#include "bbs/models.h"

#include <QList>
#include <QString>
#include <ctime>

namespace yapcr::bbs {

// スレッドの書き込み速度を算出する純関数（移植元: BaseBBS.cpp:10-17 calcSpeed）。
// count : レス数
// key   : スレッドキー（UNIX epoch 秒を表す文字列）
// now   : 現在時刻（std::time_t。テスト時は任意値を注入する）
// 戻り値: レス / 日（1日未満のスレまたは不正な key は 0.0 を返す）
double calcSpeed(int count, const QString& key, std::time_t now);

// 利便オーバーロード（now = std::time(nullptr）
double calcSpeed(int count, const QString& key);

// subject 一覧から最速の次スレ候補を選ぶ純関数（移植元: BBSOperator.cpp:515-546 getFastest）。
//
// フィルタ条件:
//   1. count >= stop のスレ（満了済み）は除外する。
//   2. epoch(key) <= curKeyEpoch のスレ（現スレより古いか同じ）は除外する。
//      curKeyEpoch == 0 の場合はフィルタしない（板URL cold-start: 全スレが対象）。
//   3. 残り候補のうち speed が最大のスレを out に書き込む。
//
// 該当なしの場合は false を返す（out は書き換えない）。
bool selectFastest(const QList<ThreadInfo>& subject,
                   int                       stop,
                   qint64                    curKeyEpoch,
                   ThreadInfo&               out);

}  // namespace yapcr::bbs
