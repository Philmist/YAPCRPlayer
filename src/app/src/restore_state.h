#pragma once

// restore_state.h — M5.5/M6: [restore] トグル × [state] 値から復元値を選択する純関数群
//
// ヘッダオンリー・純ロジック。UI / Qt Widget に依存しない。
// テストは tst_shortcuts.cpp から #include するだけで検証できる。
// clampVolume は volume_state.h から再利用する。

#include "config/config.h"
#include "display_modes.h"  // M6: FitMode（アスペクト復元）
#include "volume_state.h"

namespace yapcr::app {

// [restore].volume が true なら [state].volume を 0-100 クランプした値を返す。
// false なら fallback（既定 100）を返す。
inline int restoredVolume(const config::RestoreConfig& r,
                          const config::StateConfig&   s,
                          int fallback = 100)
{
    return r.volume ? clampVolume(s.volume) : fallback;
}

// [restore].mute が true なら [state].mute を返す。false なら fallback（既定 false）。
inline bool restoredMute(const config::RestoreConfig& r,
                         const config::StateConfig&   s,
                         bool fallback = false)
{
    return r.mute ? s.mute : fallback;
}

// 起動時に適用するウィンドウジオメトリ。
//   applyPosition: true なら (x,y) を move する。
//   applySize    : true なら (w,h) を resize する。false なら defW/defH を使う。
struct RestoredGeometry {
    bool applyPosition;
    bool applySize;
    int  x, y;
    int  w, h;
};

inline RestoredGeometry restoredGeometry(const config::RestoreConfig& r,
                                         const config::StateConfig&   s,
                                         int defW = 960, int defH = 540)
{
    return {
        r.position,
        r.size,
        s.window_x,
        s.window_y,
        r.size ? s.window_w : defW,
        r.size ? s.window_h : defH,
    };
}

// M6: FitMode 文字列変換（config [state].fit_mode との往復）。
// "inscribe" / "stretch" / "fill" / "unscaled" を相互変換する。
// 未知文字列は Inscribe にフォールバック。
inline FitMode fitModeFromString(const QString& s)
{
    if (s == QLatin1String("stretch"))  return FitMode::Stretch;
    if (s == QLatin1String("fill"))     return FitMode::Fill;
    if (s == QLatin1String("unscaled")) return FitMode::Unscaled;
    if (s == QLatin1String("aspect"))   return FitMode::AspectOverride;
    return FitMode::Inscribe;  // 既定 + フォールバック
}

inline QString fitModeToString(FitMode m)
{
    switch (m) {
        case FitMode::Stretch:        return QStringLiteral("stretch");
        case FitMode::Fill:           return QStringLiteral("fill");
        case FitMode::Unscaled:       return QStringLiteral("unscaled");
        case FitMode::AspectOverride: return QStringLiteral("aspect");
        default:                      return QStringLiteral("inscribe");
    }
}

// M6: [restore].aspect が true のときアスペクト復元値を返す。
// aspect_x/y が 0 のときはデフォルト内接（AspectDefault 相当）として扱う。
struct RestoredAspect {
    bool    apply;   // true なら復元を実施する
    FitMode fitMode;
    int     aspectX;
    int     aspectY;
};

inline RestoredAspect restoredAspect(const config::RestoreConfig& r,
                                      const config::StateConfig&   s)
{
    if (!r.aspect) {
        return {false, FitMode::Inscribe, 0, 0};
    }
    return {true, fitModeFromString(s.fit_mode), s.aspect_x, s.aspect_y};
}

} // namespace yapcr::app
