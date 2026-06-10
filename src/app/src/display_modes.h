#pragma once

#include <QList>
#include <QString>

// M4.1: フィット/アスペクトモード — mpv オプション対応付け
//
// UI 非依存の純計算ヘルパ。依存は Qt6::Core のみ。
// テスト対象: tests/tst_display (tst_display.cpp)

namespace yapcr::app {

// フィットモード（映像をウィジェット内にどう収めるか）
enum class FitMode {
    Inscribe,      // 内接（アスペクト維持・既定）: keepaspect=yes
    Stretch,       // 引き伸ばし（歪め充填）:       keepaspect=no
    Fill,          // はみ出し充填:                 panscan=1.0
    Unscaled,      // 等倍（ドットバイドット）:      video-unscaled=yes
    AspectOverride // 明示比:                       video-aspect-override=ax:ay
};

// mpv プロパティ名と値のペア
struct MpvProp {
    QString name;
    QString value;
};

// モード → mpv プロパティ完全集合（4 要素・固定順）を返す。
//
// 常に {keepaspect, panscan, video-unscaled, video-aspect-override} の
// 4 プロパティ全部を返すため、適用はべき等。モード遷移時の残留を防ぐ。
//
// aspectX/aspectY は FitMode::AspectOverride のときのみ使用。
// 不正値（0 以下）の場合は video-aspect-override=-1（内接相当）にフォールバック。
QList<MpvProp> fitModeToMpvProps(FitMode mode, int aspectX = 0, int aspectY = 0);

// アスペクト比プリセット（ハードコード既定）
// 移植元: WindowConfig::aspect (PCRPlayer/SerializeDisplay.h:89-93)
// M5: config化
struct AspectPreset {
    const char* label;  // 表示ラベル（例: "16:9"）
    int x;              // 比の分子（例: 16）
    int y;              // 比の分母（例: 9）
};

// 5 件: 16:9 / 4:3 / 5:4 / 2.35:1(=235:100) / 1.85:1(=185:100)
QList<AspectPreset> aspectPresets();

}  // namespace yapcr::app
