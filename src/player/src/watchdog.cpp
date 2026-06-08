#include "player/watchdog.h"

#include <QDebug>
#include <algorithm>

namespace yapcr::player {

Watchdog::Watchdog(QObject* parent) : QObject(parent) {}

Watchdog::~Watchdog() = default;

// ---- スロット -----------------------------------------------------------

void Watchdog::onCacheTime(double seconds, qint64 /*nowMs*/) {
    lastCacheTime_ = seconds;
}

void Watchdog::onCoreIdle(bool idle, qint64 /*nowMs*/) {
    coreIdle_ = idle;
}

void Watchdog::onFileLoaded(qint64 /*nowMs*/) {
    // 停止検知状態のみリセット（reloadCount_/bumpCount_ は維持）。
    // cache が依然止まっていれば次の evaluate でエスカレーションを継続するため。
    // 確認済み回復（confirmedRecoveryMs の healthy 再生）時のみカウンタをリセットする。
    phase_             = Phase::Idle;
    stallStartMs_      = 0;
    healthySinceMs_    = 0;
    prevEvalCacheTime_ = lastCacheTime_;
    qDebug() << "[watchdog] fileLoaded: stall リセット (reloadCount=" << reloadCount_ << ")";
}

void Watchdog::onEndFile(int reason, qint64 nowMs) {
    // reason: 0=EOF, 1=stop, 2=quit, 3=error, 4=redirect, 5=restart
    // EOF または error は供給切断とみなして即時 reload を試みる。
    // 意図的な stop (1/2) は watchdog のスコープ外。
    if ((reason == 0 || reason == 3)
            && (phase_ == Phase::Idle || phase_ == Phase::Stalling)) {
        qDebug() << "[watchdog] endFile reason=" << reason << ": 即時 reload";
        issueAction(nowMs);
    }
}

// ---- 定期評価 -----------------------------------------------------------

void Watchdog::evaluate(qint64 nowMs) {
    // cache-time が前回 evaluate から進んでいるか確認
    const bool cacheAdvancing = (lastCacheTime_ != prevEvalCacheTime_);
    prevEvalCacheTime_ = lastCacheTime_;

    switch (phase_) {
    case Phase::Idle:
    case Phase::Stalling: {
        if (coreIdle_ && !cacheAdvancing) {
            // 停止状態: stall タイマを開始・継続する
            healthySinceMs_ = 0;
            if (stallStartMs_ == 0) {
                stallStartMs_ = nowMs;
                phase_ = Phase::Stalling;
                qDebug() << "[watchdog] stall 検知開始";
            }
            if ((nowMs - stallStartMs_) >= th_.reloadAfterMs) {
                issueAction(nowMs);
            }
        } else if (!coreIdle_ && cacheAdvancing) {
            // 健全再生中: stall タイマをリセットし、回復確認タイマを進める
            if (phase_ == Phase::Stalling) {
                qDebug() << "[watchdog] stall 回復（自然回復）";
            }
            stallStartMs_ = 0;
            phase_ = Phase::Idle;
            if (healthySinceMs_ == 0) { healthySinceMs_ = nowMs; }
            if ((nowMs - healthySinceMs_) >= th_.confirmedRecoveryMs) {
                // confirmedRecoveryMs 以上 healthy が続いた = 完全回復
                if (reloadCount_ > 0 || bumpCount_ > 0) {
                    reloadCount_ = 0;
                    bumpCount_   = 0;
                    qDebug() << "[watchdog] 確認済み回復: カウンタリセット";
                }
                healthySinceMs_ = 0;
            }
        } else {
            // coreIdle_=true + cache 進行中、または coreIdle_=false + cache 停止 → 過渡状態
            // stallタイマのみリセット。healthySinceMs_ はここでは触らない
            // （neutral tick が1回入っても healthy 回復タイマが途切れないようにするため）
            stallStartMs_ = 0;
            if (phase_ == Phase::Stalling) { phase_ = Phase::Idle; }
        }
        break;
    }

    case Phase::WaitReload: {
        const qint64 backoff = backoffMs(reloadCount_);
        if ((nowMs - lastActionMs_) >= backoff) {
            // バックオフ期間中に fileLoaded が来なかった = reload 失敗
            qDebug() << "[watchdog] WaitReload タイムアウト (backoff=" << backoff << "ms)";
            issueAction(nowMs);
        }
        break;
    }

    case Phase::WaitBump: {
        // bump 後の待機: backoffMax で次アクションへ
        if ((nowMs - lastActionMs_) >= th_.backoffMaxMs) {
            qDebug() << "[watchdog] WaitBump タイムアウト";
            issueAction(nowMs);
        }
        break;
    }

    case Phase::Lost:
        // supplyLost 発行済み: 何もしない
        break;
    }
}

// ---- 内部ヘルパ ---------------------------------------------------------

qint64 Watchdog::backoffMs(int count) const {
    // count=1: backoffStep, count=2: backoffStep*2, count=3: backoffStep*4 ...
    // PCRPlayer の rebuild_ カウンタ相当
    qint64 ms = th_.backoffStepMs;
    for (int i = 1; i < count; ++i) {
        ms *= 2;
        if (ms >= th_.backoffMaxMs) { return th_.backoffMaxMs; }
    }
    return ms;
}

void Watchdog::issueAction(qint64 nowMs) {
    stallStartMs_  = 0;
    lastActionMs_  = nowMs;

    if (reloadCount_ < th_.maxReloads) {
        ++reloadCount_;
        phase_ = Phase::WaitReload;
        qDebug() << "[watchdog] reloadRequested (試行" << reloadCount_
                 << "/" << th_.maxReloads << ")";
        emit reloadRequested();
        return;
    }

    if (autoBump_ && bumpCount_ < th_.maxBumps) {
        ++bumpCount_;
        phase_ = Phase::WaitBump;
        qDebug() << "[watchdog] bumpRequested (試行" << bumpCount_
                 << "/" << th_.maxBumps << ")";
        emit bumpRequested();
        return;
    }

    // すべての試行を使い切った
    phase_ = Phase::Lost;
    qDebug() << "[watchdog] supplyLost";
    emit supplyLost();
}


}  // namespace yapcr::player
