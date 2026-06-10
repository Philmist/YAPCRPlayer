#pragma once

#include <QList>
#include <QSize>

namespace yapcr::app {

// ---- M4.0: 窓サイズ純計算ヘルパ（UI 非依存・単体テスト対象） ----------------
//
// このファイルの関数は Qt ウィジェット・mpv に依存しない純粋な算術関数であり、
// 単体テスト（tests/tst_display）でカバーする。
// ウィジェットへの適用（setFixedSize / adjustSize）は呼び出し側の責務。

// ズーム%を適用した映像の目標ピクセルサイズを返す。
// nativeW/nativeH または zoomPercent が 0 以下の場合は QSize(0,0) を返す。
//
// 算術: round(native * zoomPercent / 100) を整数演算（四捨五入・端数切り上げ）で実装。
//   式: (native * zoomPercent + 50) / 100
//
// 移植元: PCRPlayer/MainDlgSub.cpp WindowZoom() の算術部（DirectShow 依存は除去）。
QSize videoTargetForZoom(int nativeW, int nativeH, int zoomPercent);

// 明示アスペクト比を適用した表示サイズを返す（高さ据え置き・幅を調整）。
// nativeW/nativeH/aspectX/aspectY のいずれかが 0 以下の場合は QSize(0,0) を返す。
//
// 算術: width = round(nativeH * aspectX / aspectY), height = nativeH
//   式: (nativeH * aspectX + aspectY / 2) / aspectY
//
// mpv の video-aspect-override=<aspectX>:<aspectY> と整合する意味論:
//   「高さを保ちながら幅をアスペクト比に合わせる」。
//
// 移植元: PCRPlayer/Util.cpp getAspectRect() の算術を QSize で再実装。
QSize applyAspectOverride(int nativeW, int nativeH, int aspectX, int aspectY);

// ---- M4.2: プリセットテーブル（UI 非依存・単体テスト対象） -------------------

// ズーム% プリセット（ハードコード既定。// M5: config化）。
//   移植元 WindowConfig::zoom (SerializeDisplay.h:55-61)。
//   返り値: {25, 50, 75, 100, 125, 150, 200}
QList<int> zoomPresets();

// 絶対サイズ プリセット（ハードコード既定。// M5: config化）。
//   移植元 WindowConfig::size (SerializeDisplay.h:64-86) の代表値縮約セット。
//   16:9 系: 640×360 / 854×480 / 960×540 / 1280×720 / 1920×1080
//   4:3  系: 640×480 / 800×600 / 1024×768
struct SizePreset { const char* label; int w; int h; };
QList<SizePreset> sizePresets();

}  // namespace yapcr::app
