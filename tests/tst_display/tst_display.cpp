#include <QTest>

#include "window_geometry.h"
#include "display_modes.h"

// M4.0 — 窓サイズ純計算ヘルパの単体テスト
// M4.1 — フィット/アスペクトモード mpv オプション対応表の単体テスト
//
// テスト対象:
//   yapcr::app::videoTargetForZoom   — ズーム%を適用した映像目標ピクセルサイズ
//   yapcr::app::applyAspectOverride  — 明示アスペクト比を適用した表示サイズ
//   yapcr::app::fitModeToMpvProps    — フィットモード→mpv プロパティ完全集合
//   yapcr::app::aspectPresets        — アスペクトプリセット一覧
//
// mpv 実呼び出し・ウィンドウジオメトリ・UI 描画は対象外（手動 E2E）。

using yapcr::app::applyAspectOverride;
using yapcr::app::videoTargetForZoom;
using yapcr::app::FitMode;
using yapcr::app::MpvProp;
using yapcr::app::fitModeToMpvProps;
using yapcr::app::aspectPresets;

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

    // ---- fitModeToMpvProps --------------------------------------------------

    // 各モードが 4 プロパティ全部（完全集合）を正しい値で返すことを検証する
    void fitmode_full_set_each_mode() {
        // ヘルパ: QList<MpvProp> を名前→値マップに変換する
        auto toMap = [](const QList<MpvProp>& props) {
            QMap<QString, QString> m;
            for (const auto& p : props) m.insert(p.name, p.value);
            return m;
        };

        // Inscribe: 既定（keepaspect=yes / panscan=0.0 / unscaled=no / aspect-override=-1）
        {
            const auto m = toMap(fitModeToMpvProps(FitMode::Inscribe));
            QCOMPARE(m[QStringLiteral("keepaspect")],            QStringLiteral("yes"));
            QCOMPARE(m[QStringLiteral("panscan")],               QStringLiteral("0.0"));
            QCOMPARE(m[QStringLiteral("video-unscaled")],        QStringLiteral("no"));
            QCOMPARE(m[QStringLiteral("video-aspect-override")], QStringLiteral("-1"));
        }

        // Stretch: keepaspect=no、他は既定
        {
            const auto m = toMap(fitModeToMpvProps(FitMode::Stretch));
            QCOMPARE(m[QStringLiteral("keepaspect")],            QStringLiteral("no"));
            QCOMPARE(m[QStringLiteral("panscan")],               QStringLiteral("0.0"));
            QCOMPARE(m[QStringLiteral("video-unscaled")],        QStringLiteral("no"));
            QCOMPARE(m[QStringLiteral("video-aspect-override")], QStringLiteral("-1"));
        }

        // Fill: panscan=1.0、他は既定
        {
            const auto m = toMap(fitModeToMpvProps(FitMode::Fill));
            QCOMPARE(m[QStringLiteral("keepaspect")],            QStringLiteral("yes"));
            QCOMPARE(m[QStringLiteral("panscan")],               QStringLiteral("1.0"));
            QCOMPARE(m[QStringLiteral("video-unscaled")],        QStringLiteral("no"));
            QCOMPARE(m[QStringLiteral("video-aspect-override")], QStringLiteral("-1"));
        }

        // Unscaled: video-unscaled=yes、他は既定
        {
            const auto m = toMap(fitModeToMpvProps(FitMode::Unscaled));
            QCOMPARE(m[QStringLiteral("keepaspect")],            QStringLiteral("yes"));
            QCOMPARE(m[QStringLiteral("panscan")],               QStringLiteral("0.0"));
            QCOMPARE(m[QStringLiteral("video-unscaled")],        QStringLiteral("yes"));
            QCOMPARE(m[QStringLiteral("video-aspect-override")], QStringLiteral("-1"));
        }

        // 各結果は常に 4 要素
        QCOMPARE(fitModeToMpvProps(FitMode::Inscribe).size(), 4);
        QCOMPARE(fitModeToMpvProps(FitMode::Stretch).size(),  4);
        QCOMPARE(fitModeToMpvProps(FitMode::Fill).size(),     4);
        QCOMPARE(fitModeToMpvProps(FitMode::Unscaled).size(), 4);
    }

    // AspectOverride で video-aspect-override が ax:ay になり、他 3 つは既定値
    void fitmode_aspect_override() {
        auto toMap = [](const QList<MpvProp>& props) {
            QMap<QString, QString> m;
            for (const auto& p : props) m.insert(p.name, p.value);
            return m;
        };

        // 16:9
        {
            const auto m = toMap(fitModeToMpvProps(FitMode::AspectOverride, 16, 9));
            QCOMPARE(m[QStringLiteral("video-aspect-override")], QStringLiteral("16:9"));
            QCOMPARE(m[QStringLiteral("keepaspect")],            QStringLiteral("yes"));
            QCOMPARE(m[QStringLiteral("panscan")],               QStringLiteral("0.0"));
            QCOMPARE(m[QStringLiteral("video-unscaled")],        QStringLiteral("no"));
        }

        // 4:3
        {
            const auto m = toMap(fitModeToMpvProps(FitMode::AspectOverride, 4, 3));
            QCOMPARE(m[QStringLiteral("video-aspect-override")], QStringLiteral("4:3"));
        }

        // 2.35:1 = 235:100
        {
            const auto m = toMap(fitModeToMpvProps(FitMode::AspectOverride, 235, 100));
            QCOMPARE(m[QStringLiteral("video-aspect-override")], QStringLiteral("235:100"));
        }
    }

    // AspectOverride に不正値（0 以下）を渡すと -1（内接相当）にフォールバックする
    void fitmode_aspect_guard() {
        auto aspect = [](const QList<MpvProp>& props) {
            for (const auto& p : props)
                if (p.name == QStringLiteral("video-aspect-override")) return p.value;
            return QString{};
        };

        QCOMPARE(aspect(fitModeToMpvProps(FitMode::AspectOverride,  0,  9)), QStringLiteral("-1"));
        QCOMPARE(aspect(fitModeToMpvProps(FitMode::AspectOverride, 16,  0)), QStringLiteral("-1"));
        QCOMPARE(aspect(fitModeToMpvProps(FitMode::AspectOverride, -1,  9)), QStringLiteral("-1"));
        QCOMPARE(aspect(fitModeToMpvProps(FitMode::AspectOverride,  0,  0)), QStringLiteral("-1"));
    }

    // ---- aspectPresets -------------------------------------------------------

    // 5 件で x/y が 16:9 / 4:3 / 5:4 / 235:100 / 185:100
    void aspect_presets_table() {
        const auto presets = aspectPresets();
        QCOMPARE(presets.size(), 5);

        QCOMPARE(presets[0].x, 16);  QCOMPARE(presets[0].y,   9);
        QCOMPARE(presets[1].x,  4);  QCOMPARE(presets[1].y,   3);
        QCOMPARE(presets[2].x,  5);  QCOMPARE(presets[2].y,   4);
        QCOMPARE(presets[3].x, 235); QCOMPARE(presets[3].y, 100);
        QCOMPARE(presets[4].x, 185); QCOMPARE(presets[4].y, 100);
    }
};

QTEST_MAIN(TstDisplay)
#include "tst_display.moc"
