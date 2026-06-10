#include "window_geometry.h"

namespace yapcr::app {

QSize videoTargetForZoom(int nativeW, int nativeH, int zoomPercent) {
    if (nativeW <= 0 || nativeH <= 0 || zoomPercent <= 0) {
        return QSize(0, 0);
    }
    // round(native * zoom / 100) — 四捨五入（端数切り上げ）
    const int w = (nativeW * zoomPercent + 50) / 100;
    const int h = (nativeH * zoomPercent + 50) / 100;
    return QSize(w, h);
}

QSize applyAspectOverride(int nativeW, int nativeH, int aspectX, int aspectY) {
    if (nativeW <= 0 || nativeH <= 0 || aspectX <= 0 || aspectY <= 0) {
        return QSize(0, 0);
    }
    // round(nativeH * aspectX / aspectY) — 高さ据え置き・幅をアスペクト比に合わせる
    const int w = (nativeH * aspectX + aspectY / 2) / aspectY;
    return QSize(w, nativeH);
}

}  // namespace yapcr::app
