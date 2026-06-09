#pragma once

#include "bbs/models.h"

#include <QString>

namespace yapcr::bbs {

// ホスト名から板種別を判定する。
//   jbbs.shitaraba.net → BoardType::Shitaraba
//   その他             → BoardType::Jpnkn
// 移植元: BBSManager::init の match(u.host(), shitaraba) に相当（BBSManager.cpp:26）。
// M5: config 化（したらば ホスト文字列を外部設定から読む）
BoardType detectBoardType(const QString& host);

// contact URL のパース結果
struct BoardLocation {
    bool      valid{false};
    BoardType type{BoardType::Jpnkn};
    Board     board;   // scheme/host/base/board/number/code を充填済み
    Thread    thread;  // key のみ充填（title/count 等は後続 M3.x で充填）
};

// contact URL を判定・パースして BoardLocation を返す。
// valid=false なら URL 形式が合わず解析失敗。
// 移植元: BaseBBS::BaseBBS (BaseBBS.cpp:87-140)
//          ShitarabaBBS::ShitarabaBBS (BaseBBS.cpp:764-819)
BoardLocation parseContactUrl(const QString& url);

// ---------- URL ビルダー ----------
// 移植元 jpnkn:     BaseBBS::board/thread/setting/subject/dat/write (BaseBBS.cpp:689-729)
// 移植元 したらば:  ShitarabaBBS::board/thread/setting/subject/dat/write (BaseBBS.cpp:893-938)

// 板トップ URL
QString boardUrl  (const Board& b, BoardType type);
// スレッド URL（板トップから /test/read.cgi/<board>/<key>/ 等）
QString threadUrl (const Board& b, const QString& key, BoardType type);
// setting.txt の取得 URL
QString settingUrl(const Board& b, BoardType type);
// subject.txt の取得 URL
QString subjectUrl(const Board& b, BoardType type);
// dat の取得 URL（<key>.dat 等）
QString datUrl    (const Board& b, const QString& key, BoardType type);
// 書き込み先 URL（POST 先）
QString writeUrl  (const Board& b, BoardType type);

}  // namespace yapcr::bbs
