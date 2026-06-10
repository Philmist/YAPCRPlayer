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

// ---- M4.2: プリセットテーブル ---------------------------------------------------

QList<int> zoomPresets() {
    // ハードコード既定。M5: config化
    return {25, 50, 75, 100, 125, 150, 200};
}

QList<SizePreset> sizePresets() {
    // ハードコード既定。M5: config化
    // 移植元: WindowConfig::size (SerializeDisplay.h:64-86) の代表値縮約セット
    return {
        // 16:9 系
        { "640x360",    640,  360 },
        { "854x480",    854,  480 },
        { "960x540",    960,  540 },
        { "1280x720",  1280,  720 },
        { "1920x1080", 1920, 1080 },
        // 4:3 系
        { "640x480",    640,  480 },
        { "800x600",    800,  600 },
        { "1024x768",  1024,  768 },
    };
}

}  // namespace yapcr::app
