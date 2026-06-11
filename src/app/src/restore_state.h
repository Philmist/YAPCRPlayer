#pragma once

// restore_state.h — M5.5: [restore] トグル × [state] 値から復元値を選択する純関数群
//
// ヘッダオンリー・純ロジック。UI / Qt Widget に依存しない。
// テストは tst_shortcuts.cpp から #include するだけで検証できる。
// clampVolume は volume_state.h から再利用する。

#include "config/config.h"
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

} // namespace yapcr::app
