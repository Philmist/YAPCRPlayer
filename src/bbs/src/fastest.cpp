#include "bbs/fastest.h"

#include <ctime>

namespace yapcr::bbs {

// 移植元: BaseBBS.cpp:10-17
double calcSpeed(int count, const QString& key, std::time_t now)
{
    const qint64 start   = key.toLongLong();          // key = スレ立て epoch 秒
    const qint64 elapsed = static_cast<qint64>(now) - start;  // 経過秒

    if (elapsed <= 0) { return 0.0; }                 // 未来時刻 / 不正値はゼロ
    const double day = static_cast<double>(elapsed) / (60.0 * 60.0 * 24.0);

    return day > 0.0 ? static_cast<double>(count) / day : 0.0;
}

double calcSpeed(int count, const QString& key)
{
    return calcSpeed(count, key, std::time(nullptr));
}

// 移植元: BBSOperator.cpp:515-546
bool selectFastest(const QList<ThreadInfo>& subject,
                   int                       stop,
                   qint64                    curKeyEpoch,
                   ThreadInfo&               out)
{
    ThreadInfo best;  // best.speed は初期値 0.0（ThreadInfo デフォルト）

    for (const ThreadInfo& thr : subject) {
        // (a) 満了スレ除外（count >= stop）
        if (thr.count >= stop) { continue; }

        // (b) 現スレ以前（古い / 同 key）は対象外
        //     curKeyEpoch == 0 は cold-start（全スレが対象）
        const qint64 epoch = thr.key.toLongLong();
        if (curKeyEpoch != 0 && epoch <= curKeyEpoch) { continue; }

        // (c) speed 最大を採用（同速なら先着優先）
        if (thr.speed > best.speed) {
            best = thr;
        }
    }

    if (best.key.isEmpty()) { return false; }
    out = best;
    return true;
}

}  // namespace yapcr::bbs
