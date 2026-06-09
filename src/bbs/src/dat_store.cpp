#include "bbs/dat_store.h"

#include "bbs/extract.h"

#include <QHash>
#include <QList>
#include <QMap>
#include <algorithm>

namespace yapcr::bbs {

// ---------- private static ----------

// 移植元 within ラムダ（BBSManager.cpp:176-194）:
//   anchorMap の各エントリ(anchorOwner, ranges) について、
//   ranges に pos が含まれ span <= refRange なら anchorOwner を info.ref に追加。
// 「誰がこのレス(pos)を参照しているか」を info.ref へ収集する。
void DatStore::fillRef(int pos, int refRange, ResInfo& info,
                       const QMap<int, QList<Range>>& anchorMap)
{
    for (auto it = anchorMap.cbegin(); it != anchorMap.cend(); ++it) {
        const int         anchorOwner = it.key();
        const QList<Range>& ranges    = it.value();
        for (const Range& rg : ranges) {
            if (rg.span() <= refRange && rg.within(pos)) {
                info.ref.append(anchorOwner);
                break;  // このエントリは追加済み、次のエントリへ
            }
        }
    }
}

// ---------- public ----------

int DatStore::merge(const QList<ResInfo>& parsed, BoardType type, int refRange)
{
    // dedup: 既取り込み件数を超える末尾分だけを新規として取り込む（full-GET count ベース）
    const int alreadyHave = res_.size();
    if (parsed.size() <= alreadyHave) {
        return 0;
    }

    // latest 書き側（移植元 loadDat@115 + prepareDat@170-219 の組み合わせ）:
    //   初回バッチ（res_ が空）: DEFAULT_BBS_LATEST=false のため全件 latest=false（新着扱いしない）。
    //   2回目以降: 追加前に既存末尾の latest==true 連続を false へ更新（loadDat:144-148 相当）。
    const bool isInit = res_.isEmpty();
    if (!isInit) {
        for (int i = res_.size() - 1; i >= 0; --i) {
            if (!res_[i].latest) { break; }
            res_[i].latest = false;
        }
    }

    QMap<int, QList<Range>> newAnchor; // 今回の新規レスが張るアンカー

    int pos = alreadyHave + 1;  // 1始まり
    for (int i = alreadyHave; i < parsed.size(); ++i) {
        ResInfo res = parsed.at(i);

        // 初回バッチは全件 latest=false（移植元 prepareDat:216-219）
        if (isInit) {
            res.latest = false;
        }
        // 2回目以降は既定 true のまま（ResInfo の latest 既定値 = true）

        // number 充填: 空なら連番（jpnkn 用。したらば は parseDat で既に充填済み）
        if (res.number.isEmpty()) {
            res.number = QString::number(pos);
        }

        // link 充填（extractUrls は M3.3 実装済み）
        res.link = extractUrls(res.message);

        // アンカー抽出 → 今回の newAnchor に積む
        const QList<Range> ranges = extractAnchors(res.message);
        if (!ranges.isEmpty()) {
            newAnchor[pos] = ranges;
        }

        // ID 集計（空または "???" 始まりはスキップ）
        // 移植元 prepareDat:242-258 相当
        if (!res.id.isEmpty() && !res.id.startsWith(QStringLiteral("???"))) {
            idIndex_[res.id].append(pos);
            res.count = idIndex_[res.id].size();

            auto idIt = identifier_.find(res.id);
            if (idIt == identifier_.end()) {
                res.identifier = identifier_.size() + 1;
                identifier_.insert(res.id, res.identifier);
            } else {
                res.identifier = *idIt;
            }
        }

        // 既存アンカーに対して被参照更新（新規レスが既存レスを参照している場合）
        fillRef(pos, refRange, res, anchor_);

        res_.append(res);
        ++pos;
    }

    // 第2パス: total 更新 + 既存レスへ新着アンカーを反映（移植元 prepareDat:282-295）
    // total: 全レス取り込み後の ID 出現数に更新。
    // 既存レスへの反映: 新規レスが「既存レス番号」を参照していた場合に既存レスの ref を更新。
    pos = 1;
    for (ResInfo& res : res_) {
        if (!res.id.isEmpty()) {
            auto idIt = idIndex_.find(res.id);
            if (idIt != idIndex_.end()) {
                res.total = idIt->size();
            }
        }
        fillRef(pos, refRange, res, newAnchor);
        ++pos;
    }

    // anchor_ に今回の newAnchor をマージ（次回 merge の "既存" アンカーとなる）
    for (auto it = newAnchor.cbegin(); it != newAnchor.cend(); ++it) {
        anchor_.insert(it.key(), it.value());
    }

    return parsed.size() - alreadyHave;
}

void DatStore::reset()
{
    res_.clear();
    idIndex_.clear();
    identifier_.clear();
    anchor_.clear();
}

QList<ResInfo> DatStore::all() const
{
    QList<ResInfo> result;
    for (const ResInfo& res : res_) {
        if (!res.del) {
            result.append(res);
        }
    }
    return result;
}

QList<ResInfo> DatStore::byRange(Range r) const
{
    // 移植元 getExtract(range)@479: first > last を swap、境界クランプ
    if (r.first > r.last) { std::swap(r.first, r.last); }
    if (r.first < 1)            { r.first = 1; }
    if (r.last  < 1)            { r.last  = 1; }
    if (r.first > res_.size())  { return {}; }
    if (r.last  > res_.size())  { r.last = res_.size(); }

    QList<ResInfo> result;
    for (int i = r.first - 1; i < r.last; ++i) {
        if (!res_.at(i).del) {
            result.append(res_.at(i));
        }
    }
    return result;
}

QList<ResInfo> DatStore::byId(const QString& id) const
{
    auto it = idIndex_.find(id);
    if (it == idIndex_.end()) { return {}; }

    QList<ResInfo> result;
    for (int num : *it) {
        if (num >= 1 && num <= res_.size()) {
            result.append(res_.at(num - 1));
        }
    }
    return result;
}

QList<ResInfo> DatStore::byRef(int resNumber) const
{
    // 移植元 getExtract(ref)@521:
    //   resNumber のレスの ref 一覧 = そのレスを参照しているレス群 → それらの ResInfo を返す
    if (resNumber < 1 || resNumber > res_.size()) { return {}; }
    const QList<int>& refs = res_.at(resNumber - 1).ref;
    if (refs.isEmpty()) { return {}; }

    QList<ResInfo> result;
    for (int num : refs) {
        if (num >= 1 && num <= res_.size()) {
            result.append(res_.at(num - 1));
        }
    }
    return result;
}

QList<ResInfo> DatStore::byNumbers(const QList<int>& nums) const
{
    QList<ResInfo> result;
    for (int num : nums) {
        if (num >= 1 && num <= res_.size()) {
            result.append(res_.at(num - 1));
        }
    }
    return result;
}

int DatStore::count() const
{
    return res_.size();
}

bool DatStore::isEmpty() const
{
    return res_.isEmpty();
}

}  // namespace yapcr::bbs
