#pragma once

#include "bbs/models.h"

#include <QHash>
#include <QList>
#include <QMap>
#include <QString>

namespace yapcr::bbs {

// レス列横断集計コア（QObject 非依存・ネットワーク非依存）。
// 移植元: BBSManager::prepareDat(@170)/cleanDat(@304)/getExtract 群(@439-553) 相当。
//
// M3.4 が担う: number(jpnkn 連番) / link / ref / count / total / identifier / latest(書き側) の充填。
// M3.6 に委譲: getDat(diff,len) 系の読み側差分ウィンドウ / getPos / cleanDat。
// 専ブラ表示オミット: NG置換・HTMLエスケープ・split は実装しない（project-direction）。
class DatStore {
public:
    // 移植元 DEFAULT_BBS_RANGE=5（BBSOperator.h:16）。// M5: config 化
    static constexpr int kDefaultRefRange = 5;

    DatStore() = default;

    // parseDat() の全件結果を渡し、未取り込みの末尾分のみ新規レスとして merge する。
    // count ベース dedup = full-GET 前提（partial バイト取得は M3.x 以降）。
    // 戻り値 = 今回追加された件数（0 = 新着なし）。
    // refRange: アンカー範囲の幅がこれ以下のときのみ被参照に数える（>>1-1000 等の広域除外）。
    // 移植元: BBSManager::prepareDat(@170)
    int  merge(const QList<ResInfo>& parsed, BoardType type,
               int refRange = kDefaultRefRange);

    // スレッド切替時に全データをクリアする。
    // 移植元: BBSManager::change(@62) 内の dat_.reset()/pos_.reset()
    void reset();

    // ---- extract 系クエリ（移植元 getExtract 群 @439-553 相当） ----

    // del 除外の全レスを返す（移植元 getExtract()@439）。
    QList<ResInfo> all() const;

    // 範囲指定（移植元 getExtract(range)@479）。
    // r.first > r.last は自動 swap。範囲は res_ のサイズにクランプ。del は除外。
    QList<ResInfo> byRange(Range r) const;

    // ID 一致レスを返す（移植元 getExtract(id)@503）。未知 ID は空リスト。
    QList<ResInfo> byId(const QString& id) const;

    // resNumber を参照しているレスを返す（移植元 getExtract(ref)@521）。
    // resNumber: 1 始まり。ResInfo::ref は「このレスを参照しているレス番号の一覧」。
    QList<ResInfo> byRef(int resNumber) const;

    // レス番号リスト指定（移植元 getExtract(vec)@539）。1 始まり。範囲外インデックスは無視。
    QList<ResInfo> byNumbers(const QList<int>& nums) const;

    int  count() const;
    bool isEmpty() const;

private:
    // アンカーマップを参照し、pos が参照先に含まれるエントリを info.ref に追記する。
    // 移植元 within ラムダ（BBSManager.cpp:176-194）相当。
    // anchorMap: anchorOwner → [アンカーが指す Range 群]
    // pos が Range 内かつ span <= refRange のとき anchorOwner を info.ref に追加。
    static void fillRef(int pos, int refRange, ResInfo& info,
                        const QMap<int, QList<Range>>& anchorMap);

    QList<ResInfo>             res_;        // 全レス（del フラグ込み、1始まりインデックス）
    QHash<QString, QList<int>> idIndex_;    // ID → レス番号(1始まり)群
    QHash<QString, int>        identifier_; // ID → 連番（1始まり、ID 初出順）
    QMap<int, QList<Range>>    anchor_;     // レス番号 → そのレスが張るアンカー範囲群
};

}  // namespace yapcr::bbs
