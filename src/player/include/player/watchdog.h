#pragma once

#include <QObject>
#include <QtGlobal>

namespace yapcr::player {

// mpv 供給停止を検知し、自動復帰アクション（reload / bump）を要求する純ロジッククラス。
// mpv API / Qt ネットワーク等に一切依存しない。
//
// 設計:
//   全ての時刻は外部から nowMs（ms since epoch）として注入する。
//   本番: SessionController の QTimer (≈3s 間隔) から evaluate() を駆動し、
//         MpvBackend のプロパティシグナルを on*() スロットへ接続する。
//   テスト: 任意の nowMs を注入して決定論的に停止検知を検証できる。
//
// 停止判定:
//   core-idle==true かつ demuxer-cache-time が進まない状態が reloadAfterMs 継続 → 停止とみなす。
//
// 復帰フロー:
//   1. reloadRequested を emit → SessionController が mpv に loadfile を再発行。
//   2. maxReloads 回 reload しても復帰しなければ bumpRequested を emit。
//      (autoBump==false の場合は reload のみ繰り返す)
//   3. maxBumps 回 bump しても復帰しなければ supplyLost を emit して停止。
//
// バックオフ:
//   連続 reload は backoffStep の指数倍（上限 backoffMax）で間隔を広げる。
class Watchdog : public QObject {
    Q_OBJECT

public:
    // 閾値・試行回数の設定（テストで差し替え可能）
    struct Thresholds {
        qint64 reloadAfterMs{10'000};         // 停止と判定してリロードを発行するまでの時間
        qint64 backoffStepMs{10'000};         // バックオフ初期値 (count=1: 10s, 2: 20s, 3: 40s, ...)
        qint64 backoffMaxMs{60'000};          // バックオフ上限
        int    maxReloads{3};                 // reload 試行上限（超えたら bump へ）
        int    maxBumps{2};                   // bump 試行上限（超えたら supplyLost）
        qint64 confirmedRecoveryMs{60'000};   // この間 healthy 再生が続いたら retry カウンタをリセット
    };

    explicit Watchdog(QObject* parent = nullptr);
    ~Watchdog() override;

    // 自動 bump を有効/無効にする（既定: 有効）。
    // false のとき bumpRequested は発行しない（reload のみ繰り返す）。
    void setAutoBump(bool enabled) { autoBump_ = enabled; }
    bool autoBump() const          { return autoBump_; }

    // テスト用: 閾値を上書きする。
    void setThresholds(const Thresholds& th) { th_ = th; }
    const Thresholds& thresholds() const     { return th_; }

public slots:
    // demuxer-cache-time の更新。MpvBackend::cacheTimeChanged から接続する。
    void onCacheTime(double seconds, qint64 nowMs);

    // core-idle フラグの更新。MpvBackend::coreIdleChanged から接続する。
    void onCoreIdle(bool idle, qint64 nowMs);

    // 再生開始（mpv FILE_LOADED）。復帰成功として全状態をリセットする。
    void onFileLoaded(qint64 nowMs);

    // 再生終了（mpv END_FILE）。reason=0(EOF) または 3(error) は即時 reload を試みる。
    void onEndFile(int reason, qint64 nowMs);

    // 定期評価（SessionController の QTimer から呼ぶ）。
    // 現在の状態に応じて reloadRequested / bumpRequested / supplyLost を emit する。
    void evaluate(qint64 nowMs);

signals:
    // mpv loadfile の再発行を要求する。SessionController が streamUrl_ で load() を呼ぶ。
    void reloadRequested();

    // PeerCast bump の実行を要求する。SessionController が PeerCastController::bump() を呼ぶ。
    void bumpRequested();

    // 自動復帰の試行を使い切った。SessionController がユーザーに通知する。
    void supplyLost();

private:
    enum class Phase {
        Idle,        // 正常再生中（または未開始）
        Stalling,    // 停止検知中（stallStartMs_ からの経過時間を計測）
        WaitReload,  // reload 発行後・復帰待ち（lastActionMs_ からのバックオフを計測）
        WaitBump,    // bump 発行後・復帰待ち
        Lost,        // supplyLost 発行済み（これ以上のアクションを起こさない）
    };

    // reloadCount_ に基づくバックオフ時間を返す（指数増加・上限付き）
    qint64 backoffMs(int count) const;

    // 状態に応じて reload / bump / supplyLost を emit し、フェーズを遷移させる
    void issueAction(qint64 nowMs);

    Phase   phase_{Phase::Idle};
    bool    coreIdle_{false};
    double  lastCacheTime_{0.0};     // onCacheTime で更新される最新値
    double  prevEvalCacheTime_{0.0}; // evaluate 前回の lastCacheTime_ スナップショット
    qint64  stallStartMs_{0};        // 停止が始まったと判定した時刻（0=未検知）
    qint64  lastActionMs_{0};        // 最後に reload/bump を発行した時刻
    qint64  healthySinceMs_{0};      // 健全再生が始まった時刻（0=未計測）
    int     reloadCount_{0};         // 累積 reload 試行回数
    int     bumpCount_{0};           // 累積 bump 試行回数
    bool    autoBump_{true};
    Thresholds th_;
};

}  // namespace yapcr::player
