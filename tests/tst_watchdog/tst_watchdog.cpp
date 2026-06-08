#include <QSignalSpy>
#include <QTest>

#include "player/watchdog.h"

// テスト用の nowMs 開始点（絶対値は不問、差分だけが重要）
static constexpr qint64 T0 = 1'000'000;

// ---- タイムライン解説 -------------------------------------------------------
// evaluate() はペア比較（prevEvalCacheTime_ vs lastCacheTime_）で cache 進行を検出する。
// このため最初の evaluate() 呼び出しは「ベースライン確定」に使われ、
// 2 回目以降の evaluate() から停止判定が有効になる。
//
// fastThresholds: reloadAfter=1s / backoffStep=500ms / backoffMax=2s / maxReloads=3 / maxBumps=2
//
// 停止→reload の基本タイムライン（1.0 で固定の cacheTime を使う場合）:
//   evaluate(T0)      : prevEval=0→1.0, advancing=true → 停止なし
//   evaluate(T0+100)  : prevEval=1.0, advancing=false → stallStart=T0+100
//   evaluate(T0+1100) : elapsed=1000ms ≥ 1s → reload1 (count=1, backoff=500ms)
//   evaluate(T0+1600) : WaitReload elapsed=500ms → reload2 (count=2, backoff=1000ms)
//   evaluate(T0+2600) : WaitReload elapsed=1000ms → reload3 (count=3, backoff=2000ms)
//   evaluate(T0+4600) : WaitReload elapsed=2000ms → bump1 (bumpCount=1)
//   evaluate(T0+6600) : WaitBump  elapsed=2000ms → bump2 (bumpCount=2)
//   evaluate(T0+8600) : WaitBump  elapsed=2000ms → supplyLost
// ---------------------------------------------------------------------------

class TstWatchdog : public QObject {
    Q_OBJECT

private:
    static yapcr::player::Watchdog::Thresholds fastThresholds() {
        return {
            .reloadAfterMs = 1'000,
            .backoffStepMs = 500,
            .backoffMaxMs  = 2'000,
            .maxReloads    = 3,
            .maxBumps      = 2,
        };
    }

    static std::unique_ptr<yapcr::player::Watchdog> makeWatchdog() {
        auto wd = std::make_unique<yapcr::player::Watchdog>();
        wd->setThresholds(fastThresholds());
        return wd;
    }

    // coreIdle=true, cacheTime=固定値（停止中）を注入する
    static void feedStall(yapcr::player::Watchdog& wd, qint64 nowMs,
                          double cacheVal = 1.0) {
        wd.onCoreIdle(true,    nowMs);
        wd.onCacheTime(cacheVal, nowMs);
    }

private slots:

    // ---- 停止検知 → reloadRequested ----------------------------------------

    void stall_triggers_reload_after_threshold() {
        auto wd = makeWatchdog();
        QSignalSpy spyReload(wd.get(), &yapcr::player::Watchdog::reloadRequested);

        feedStall(*wd, T0);
        wd->evaluate(T0);       // ベースライン確定: advancing=true（1.0!=0.0）, 停止なし
        QCOMPARE(spyReload.count(), 0);

        wd->evaluate(T0 + 100); // advancing=false → stallStart=T0+100, elapsed=0
        QCOMPARE(spyReload.count(), 0);

        // reloadAfterMs=1000ms 経過 → reload
        wd->evaluate(T0 + 1'100);
        QCOMPARE(spyReload.count(), 1);
    }

    void no_stall_if_cache_advancing() {
        auto wd = makeWatchdog();
        QSignalSpy spyReload(wd.get(), &yapcr::player::Watchdog::reloadRequested);

        wd->onCoreIdle(true, T0);
        // cache-time が定期的に増加し続ける → 停止なし
        for (int i = 0; i <= 5; ++i) {
            wd->onCacheTime(static_cast<double>(i), T0 + static_cast<qint64>(i) * 500);
            wd->evaluate(T0 + static_cast<qint64>(i) * 500);
        }
        QCOMPARE(spyReload.count(), 0);
    }

    void no_stall_if_core_not_idle() {
        auto wd = makeWatchdog();
        QSignalSpy spyReload(wd.get(), &yapcr::player::Watchdog::reloadRequested);

        // cache-time は固定だが core-idle=false → 正常再生中とみなす
        wd->onCoreIdle(false, T0);
        wd->onCacheTime(1.0, T0);
        wd->evaluate(T0);
        wd->evaluate(T0 + 100);
        wd->evaluate(T0 + 2'000);
        QCOMPARE(spyReload.count(), 0);
    }

    // ---- バックオフ ----------------------------------------------------------

    void reload_backoff_increases() {
        auto wd = makeWatchdog();
        QSignalSpy spyReload(wd.get(), &yapcr::player::Watchdog::reloadRequested);

        feedStall(*wd, T0);
        wd->evaluate(T0);        // ベースライン
        wd->evaluate(T0 + 100);  // stall 検知開始

        // reload1 at T0+1100 (elapsed=1000ms, backoff=500ms)
        wd->evaluate(T0 + 1'100);
        QCOMPARE(spyReload.count(), 1);

        // バックオフ (500ms) 未満では発火しない
        wd->evaluate(T0 + 1'350);
        QCOMPARE(spyReload.count(), 1);

        // reload2 at T0+1600 (elapsed=500ms ≥ 500ms, backoff=1000ms)
        wd->evaluate(T0 + 1'600);
        QCOMPARE(spyReload.count(), 2);

        // バックオフ (1000ms) 未満では発火しない
        wd->evaluate(T0 + 2'000);
        QCOMPARE(spyReload.count(), 2);

        // reload3 at T0+2600 (elapsed=1000ms ≥ 1000ms)
        wd->evaluate(T0 + 2'600);
        QCOMPARE(spyReload.count(), 3);
    }

    // ---- reload → bump エスカレーション -----------------------------------

    void escalates_to_bump_after_max_reloads() {
        auto wd = makeWatchdog();
        QSignalSpy spyReload(wd.get(), &yapcr::player::Watchdog::reloadRequested);
        QSignalSpy spyBump(wd.get(),   &yapcr::player::Watchdog::bumpRequested);

        feedStall(*wd, T0);
        wd->evaluate(T0);
        wd->evaluate(T0 + 100);
        wd->evaluate(T0 + 1'100);  // reload1
        wd->evaluate(T0 + 1'600);  // reload2
        wd->evaluate(T0 + 2'600);  // reload3 (count=maxReloads)

        QCOMPARE(spyReload.count(), 3);
        QCOMPARE(spyBump.count(),   0);

        // backoff(3)=2000ms (上限) 後 → bump へエスカレーション
        wd->evaluate(T0 + 4'600);
        QCOMPARE(spyReload.count(), 3);
        QCOMPARE(spyBump.count(),   1);
    }

    // ---- autoBump=false 時は bump を発行せず supplyLost ----------------------

    void no_auto_bump_when_disabled() {
        auto wd = makeWatchdog();
        wd->setAutoBump(false);
        QSignalSpy spyBump(wd.get(),   &yapcr::player::Watchdog::bumpRequested);
        QSignalSpy spyLost(wd.get(),   &yapcr::player::Watchdog::supplyLost);

        feedStall(*wd, T0);
        wd->evaluate(T0);
        wd->evaluate(T0 + 100);
        wd->evaluate(T0 + 1'100);  // reload1
        wd->evaluate(T0 + 1'600);  // reload2
        wd->evaluate(T0 + 2'600);  // reload3

        // backoff(3)=2000ms 後 → autoBump=false なので bump せず supplyLost
        wd->evaluate(T0 + 4'600);

        QCOMPARE(spyBump.count(), 0);
        QCOMPARE(spyLost.count(), 1);
    }

    // ---- supplyLost ----------------------------------------------------------

    void supply_lost_after_max_bumps() {
        auto wd = makeWatchdog();
        QSignalSpy spyBump(wd.get(), &yapcr::player::Watchdog::bumpRequested);
        QSignalSpy spyLost(wd.get(), &yapcr::player::Watchdog::supplyLost);

        feedStall(*wd, T0);
        wd->evaluate(T0);
        wd->evaluate(T0 + 100);
        wd->evaluate(T0 + 1'100);  // reload1
        wd->evaluate(T0 + 1'600);  // reload2
        wd->evaluate(T0 + 2'600);  // reload3
        wd->evaluate(T0 + 4'600);  // bump1 (WaitBump)
        wd->evaluate(T0 + 6'600);  // bump2 (WaitBump elapsed=2000ms)

        QCOMPARE(spyBump.count(), 2);
        QCOMPARE(spyLost.count(), 0);

        // backoffMax=2000ms 後 → supplyLost
        wd->evaluate(T0 + 8'600);
        QCOMPARE(spyLost.count(), 1);
    }

    void supply_lost_stops_further_actions() {
        auto wd = makeWatchdog();
        QSignalSpy spyReload(wd.get(), &yapcr::player::Watchdog::reloadRequested);
        QSignalSpy spyBump(wd.get(),   &yapcr::player::Watchdog::bumpRequested);
        QSignalSpy spyLost(wd.get(),   &yapcr::player::Watchdog::supplyLost);

        feedStall(*wd, T0);
        wd->evaluate(T0);
        wd->evaluate(T0 + 100);
        wd->evaluate(T0 + 1'100);
        wd->evaluate(T0 + 1'600);
        wd->evaluate(T0 + 2'600);
        wd->evaluate(T0 + 4'600);
        wd->evaluate(T0 + 6'600);
        wd->evaluate(T0 + 8'600);  // supplyLost

        QCOMPARE(spyLost.count(), 1);
        const int r = spyReload.count();
        const int b = spyBump.count();

        // Lost 状態でさらに evaluate しても何も増えない
        wd->evaluate(T0 + 20'000);
        wd->evaluate(T0 + 30'000);
        QCOMPARE(spyReload.count(), r);
        QCOMPARE(spyBump.count(),   b);
        QCOMPARE(spyLost.count(),   1);
    }

    // ---- fileLoaded で stall 状態をリセット（カウンタは維持）----------------

    void file_loaded_resets_stall_state() {
        auto wd = makeWatchdog();
        QSignalSpy spyReload(wd.get(), &yapcr::player::Watchdog::reloadRequested);

        feedStall(*wd, T0);
        wd->evaluate(T0);
        wd->evaluate(T0 + 100);
        wd->evaluate(T0 + 1'100);  // reload1 発火
        QCOMPARE(spyReload.count(), 1);

        // fileLoaded → stall 状態をリセット（reloadCount=1 維持、phase=Idle）
        wd->onFileLoaded(T0 + 1'200);

        // リセット直後: prevEvalCacheTime_=lastCacheTime_=1.0 に揃えているため
        // 最初の evaluate で即 advancing=false (停止検知開始)
        feedStall(*wd, T0 + 1'300);
        wd->evaluate(T0 + 1'300);   // stallStart=T0+1300, elapsed=0
        QCOMPARE(spyReload.count(), 1);

        wd->evaluate(T0 + 1'400);   // elapsed=100ms < 1000ms → 発火なし
        QCOMPARE(spyReload.count(), 1);

        wd->evaluate(T0 + 2'300);   // elapsed=1000ms → reload2 (reloadCount_=1→2)
        // reloadCount_=1 で maxReloads=3 未満のため reload を継続
        QCOMPARE(spyReload.count(), 2);
    }

    // ---- fileLoaded が挟まってもエスカレーションが続く ---------------------
    // 「接続はできるが cache が止まったまま」のケース: onFileLoaded を毎回挟んでも
    // reloadCount_ は維持され、maxReloads 後に bumpRequested が発火すること。

    void file_loaded_does_not_reset_escalation_counters() {
        auto wd = makeWatchdog();
        QSignalSpy spyReload(wd.get(), &yapcr::player::Watchdog::reloadRequested);
        QSignalSpy spyBump  (wd.get(), &yapcr::player::Watchdog::bumpRequested);

        // ベースライン
        feedStall(*wd, T0);
        wd->evaluate(T0);         // prevEval=0→1.0, advancing=true → neutral
        wd->evaluate(T0 + 100);   // advancing=false → stallStart=T0+100

        // reload1
        wd->evaluate(T0 + 1'100);     // elapsed=1000ms → reload1 (reloadCount_=1)
        QCOMPARE(spyReload.count(), 1);
        wd->onFileLoaded(T0 + 1'200); // stall リセット・reloadCount_=1 維持

        // reload2: fileLoaded 後も cache は固定のまま
        feedStall(*wd, T0 + 1'300);
        wd->evaluate(T0 + 1'300);     // stall 検知開始 stallStart=T0+1300
        wd->evaluate(T0 + 2'300);     // elapsed=1000ms → reload2 (reloadCount_=2)
        QCOMPARE(spyReload.count(), 2);
        wd->onFileLoaded(T0 + 2'400); // stall リセット・reloadCount_=2 維持

        // reload3
        feedStall(*wd, T0 + 2'500);
        wd->evaluate(T0 + 2'500);     // stall 検知開始 stallStart=T0+2500
        wd->evaluate(T0 + 3'500);     // elapsed=1000ms → reload3 (reloadCount_=3=maxReloads)
        QCOMPARE(spyReload.count(), 3);
        wd->onFileLoaded(T0 + 3'600); // stall リセット・reloadCount_=3 維持

        // reload 上限到達 → 次回は bump へエスカレーション
        feedStall(*wd, T0 + 3'700);
        wd->evaluate(T0 + 3'700);     // stall 検知開始 stallStart=T0+3700
        wd->evaluate(T0 + 4'700);     // elapsed=1000ms → reloadCount_≥maxReloads → bump1
        QCOMPARE(spyReload.count(), 3);  // reload はこれ以上増えない
        QCOMPARE(spyBump.count(),   1);
    }

    // ---- confirmedRecoveryMs 後にカウンタがリセットされる ----------------------
    // 仕様: !coreIdle_ && cache が進んでいる状態が confirmedRecoveryMs 以上続いた場合、
    //       reloadCount_/bumpCount_ をリセットし「新エピソード」として扱う。

    void confirmed_recovery_resets_counters() {
        // discriminating 条件: reloadCount_=maxReloads=3 まで積み上げた後に回復させる。
        // reset なし → 次の stall は bump。reset あり → reload から再開。
        auto wd = makeWatchdog();
        auto th = fastThresholds();
        th.confirmedRecoveryMs = 3'000;
        wd->setThresholds(th);
        QSignalSpy spyReload(wd.get(), &yapcr::player::Watchdog::reloadRequested);
        QSignalSpy spyBump  (wd.get(), &yapcr::player::Watchdog::bumpRequested);

        // stall→reload→fileLoaded を 3 回繰り返し reloadCount_=3=maxReloads にする
        feedStall(*wd, T0);
        wd->evaluate(T0);
        wd->evaluate(T0 + 100);      // stall 開始
        wd->evaluate(T0 + 1'100);    // reload1 (reloadCount_=1)
        wd->onFileLoaded(T0 + 1'200);

        feedStall(*wd, T0 + 1'300);
        wd->evaluate(T0 + 1'300);    // prevEval=1.0=lastCache → 即 stall 開始
        wd->evaluate(T0 + 2'300);    // reload2 (reloadCount_=2)
        wd->onFileLoaded(T0 + 2'400);

        feedStall(*wd, T0 + 2'500);
        wd->evaluate(T0 + 2'500);    // stall 開始
        wd->evaluate(T0 + 3'500);    // reload3 (reloadCount_=3=maxReloads)
        wd->onFileLoaded(T0 + 3'600);

        QCOMPARE(spyReload.count(), 3);
        QCOMPARE(spyBump.count(),   0);

        // 健全再生を confirmedRecoveryMs=3000ms 維持 → カウンタリセット
        // T0+3700 から 500ms×7 tick (i=0..6) → T0+6700 で elapsed=3000ms
        for (int i = 0; i <= 6; ++i) {
            const qint64 t = T0 + 3'700 + i * 500;
            wd->onCoreIdle(false, t);
            wd->onCacheTime(10.0 + i, t);
            wd->evaluate(t);  // i=6 (T0+6700): elapsed=3000ms → RESET
        }

        // 新 stall: reset が機能していれば bump ではなく reload が発火する
        // (cache が 16.0 から 1.0 に変わるので最初の evaluate は advancing=true → neutral)
        feedStall(*wd, T0 + 7'200);
        wd->evaluate(T0 + 7'200);      // neutral (1.0 vs 16.0)
        wd->evaluate(T0 + 7'300);      // advancing=false → stall 開始
        wd->evaluate(T0 + 8'300);      // reload (reloadCount_=0→1)

        QCOMPARE(spyBump.count(),   0);  // reset なければ bump が発火: discriminating assertion
        QCOMPARE(spyReload.count(), 4);  // 3 + 1
    }

    // ---- neutral tick が1回入っても recovery タイマは途切れない ----------------
    // else ブランチ（過渡状態）では healthySinceMs_ をリセットしないことを確認する。
    // 同じく reloadCount_=3 まで積み上げて、neutral tick を挟んでもリセットが発生することで検証する。

    void neutral_tick_does_not_interrupt_recovery_timer() {
        auto wd = makeWatchdog();
        auto th = fastThresholds();
        th.confirmedRecoveryMs = 3'000;
        wd->setThresholds(th);
        QSignalSpy spyReload(wd.get(), &yapcr::player::Watchdog::reloadRequested);
        QSignalSpy spyBump  (wd.get(), &yapcr::player::Watchdog::bumpRequested);

        // reloadCount_=3 まで積み上げ（confirmed_recovery_resets_counters と同様）
        feedStall(*wd, T0);
        wd->evaluate(T0);
        wd->evaluate(T0 + 100);
        wd->evaluate(T0 + 1'100);    // reload1
        wd->onFileLoaded(T0 + 1'200);

        feedStall(*wd, T0 + 1'300);
        wd->evaluate(T0 + 1'300);
        wd->evaluate(T0 + 2'300);    // reload2
        wd->onFileLoaded(T0 + 2'400);

        feedStall(*wd, T0 + 2'500);
        wd->evaluate(T0 + 2'500);
        wd->evaluate(T0 + 3'500);    // reload3 (reloadCount_=3=maxReloads)
        wd->onFileLoaded(T0 + 3'600);

        QCOMPARE(spyReload.count(), 3);

        // 健全再生の途中に neutral tick (coreIdle_=true + cache 進行) を1回挿入する
        // healthySinceMs_ が neutral tick でリセットされなければ confirmedRecoveryMs に到達できる

        // T0+3700〜T0+4700: healthy 2 ticks
        wd->onCoreIdle(false, T0 + 3'700); wd->onCacheTime(10.0, T0 + 3'700); wd->evaluate(T0 + 3'700);
        wd->onCoreIdle(false, T0 + 4'200); wd->onCacheTime(11.0, T0 + 4'200); wd->evaluate(T0 + 4'200);

        // T0+4700: neutral tick (coreIdle_=true, cache 進行)
        wd->onCoreIdle(true,  T0 + 4'700); wd->onCacheTime(12.0, T0 + 4'700); wd->evaluate(T0 + 4'700);

        // T0+5200〜T0+6700: healthy 再開 (T0+6700 で elapsed=3000ms → RESET)
        for (int i = 0; i <= 3; ++i) {
            const qint64 t = T0 + 5'200 + i * 500;
            wd->onCoreIdle(false, t);
            wd->onCacheTime(13.0 + i, t);
            wd->evaluate(t);  // i=3 → T0+6700: elapsed=T0+6700-T0+3700=3000ms → RESET
        }

        // 新 stall → neutral tick を挟んでも reset が機能していれば bump ではなく reload
        feedStall(*wd, T0 + 7'200);
        wd->evaluate(T0 + 7'200);      // neutral
        wd->evaluate(T0 + 7'300);      // stall 開始
        wd->evaluate(T0 + 8'300);      // reload

        QCOMPARE(spyBump.count(),   0);  // discriminating assertion
        QCOMPARE(spyReload.count(), 4);
    }

    // ---- endFile(error) で即時 reload ----------------------------------------

    void end_file_error_triggers_immediate_reload() {
        auto wd = makeWatchdog();
        QSignalSpy spyReload(wd.get(), &yapcr::player::Watchdog::reloadRequested);

        // reason=3 (error) → stall タイマを待たずに即時 reload
        wd->onEndFile(3, T0);
        QCOMPARE(spyReload.count(), 1);
    }

    void end_file_eof_triggers_immediate_reload() {
        auto wd = makeWatchdog();
        QSignalSpy spyReload(wd.get(), &yapcr::player::Watchdog::reloadRequested);

        wd->onEndFile(0, T0);
        QCOMPARE(spyReload.count(), 1);
    }

    void end_file_stop_does_not_trigger_reload() {
        auto wd = makeWatchdog();
        QSignalSpy spyReload(wd.get(), &yapcr::player::Watchdog::reloadRequested);

        // reason=1 (意図的な stop) → watchdog は何もしない
        wd->onEndFile(1, T0);
        QCOMPARE(spyReload.count(), 0);
    }
};

QTEST_MAIN(TstWatchdog)
#include "tst_watchdog.moc"
