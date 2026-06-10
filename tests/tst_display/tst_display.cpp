#include <QTest>

#include "window_geometry.h"

// M4.0 — 窓サイズ純計算ヘルパの単体テスト
//
// テスト対象:
//   yapcr::app::videoTargetForZoom   — ズーム%を適用した映像目標ピクセルサイズ
//   yapcr::app::applyAspectOverride  — 明示アスペクト比を適用した表示サイズ
//
// mpv 実呼び出し・ウィンドウジオメトリ・UI 描画は対象外（手動 E2E）。

using yapcr::app::applyAspectOverride;
using yapcr::app::videoTargetForZoom;

class TstDisplay : public QObject {
    Q_OBJECT

private slots:
    // ---- videoTargetForZoom -------------------------------------------------

    void zoom_standard_1280x720() {
        // 1280×720 を標準プリセット % で適用する
        QCOMPARE(videoTargetForZoom(1280, 720,  25), QSize( 320,  180));
        QCOMPARE(videoTargetForZoom(1280, 720,  50), QSize( 640,  360));
        QCOMPARE(videoTargetForZoom(1280, 720,  75), QSize( 960,  540));
        QCOMPARE(videoTargetForZoom(1280, 720, 100), QSize(1280,  720));
        QCOMPARE(videoTargetForZoom(1280, 720, 125), QSize(1600,  900));
        QCOMPARE(videoTargetForZoom(1280, 720, 150), QSize(1920, 1080));
        QCOMPARE(videoTargetForZoom(1280, 720, 200), QSize(2560, 1440));
    }

    void zoom_fractional_rounding() {
        // 333×187 at 150%: 333*1.5=499.5→500, 187*1.5=280.5→281
        QCOMPARE(videoTargetForZoom(333, 187, 150), QSize(500, 281));

        // 1×1 at 75%: 0.75→1（端数は切り上げ）
        QCOMPARE(videoTargetForZoom(1, 1, 75), QSize(1, 1));

        // 1×1 at 49%: 0.49→0
        QCOMPARE(videoTargetForZoom(1, 1, 49), QSize(0, 0));
        // ガード(0以下)ではなく算術結果が 0 になる場合→ QSize(0,0) が返る
        // 注: zoomPercent=49 は正値なのでガードは通過するが計算結果が 0
        //     実装: (1*49+50)/100=99/100=0 → QSize(0,0) を返す
    }

    void zoom_guard() {
        // native が 0 以下
        QCOMPARE(videoTargetForZoom(  0, 720, 100), QSize(0, 0));
        QCOMPARE(videoTargetForZoom(1280,  0, 100), QSize(0, 0));
        QCOMPARE(videoTargetForZoom( -1, 720, 100), QSize(0, 0));
        // zoomPercent が 0 以下
        QCOMPARE(videoTargetForZoom(1280, 720,   0), QSize(0, 0));
        QCOMPARE(videoTargetForZoom(1280, 720,  -1), QSize(0, 0));
    }

    // ---- applyAspectOverride ------------------------------------------------

    void aspect_standard_ratios_height720() {
        // 高さ=720 を基準に各アスペクト比の幅を計算する
        // 16:9 → 1280
        QCOMPARE(applyAspectOverride(1280, 720, 16, 9), QSize(1280, 720));
        // 4:3 → 960
        QCOMPARE(applyAspectOverride(1280, 720,  4, 3), QSize(960, 720));
        // 5:4 → 900
        QCOMPARE(applyAspectOverride(1280, 720,  5, 4), QSize(900, 720));
        // 2.35:1 = 235:100 → 1692
        QCOMPARE(applyAspectOverride(1280, 720, 235, 100), QSize(1692, 720));
        // 1.85:1 = 185:100 → 1332
        QCOMPARE(applyAspectOverride(1280, 720, 185, 100), QSize(1332, 720));
    }

    void aspect_height_preserved() {
        // 高さは nativeH をそのまま返す（nativeW は使わない）
        const QSize r = applyAspectOverride(999, 480, 16, 9);
        QCOMPARE(r.height(), 480);
        // 480 * 16 / 9 = 853.3...→ round = 853
        QCOMPARE(r.width(), 853);
    }

    void aspect_guard() {
        QCOMPARE(applyAspectOverride(  0, 720, 16,  9), QSize(0, 0));
        QCOMPARE(applyAspectOverride(1280,  0, 16,  9), QSize(0, 0));
        QCOMPARE(applyAspectOverride(1280, 720,  0,  9), QSize(0, 0));
        QCOMPARE(applyAspectOverride(1280, 720, 16,  0), QSize(0, 0));  // 0 除算ガード
        QCOMPARE(applyAspectOverride( -1, 720, 16,  9), QSize(0, 0));
    }
};

QTEST_MAIN(TstDisplay)
#include "tst_display.moc"
