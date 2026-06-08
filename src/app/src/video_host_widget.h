#pragma once

#include <QWidget>

namespace yapcr::app {

// mpv の --wid 埋め込み先となるネイティブウィジェット。
//
// WA_NativeWindow で OS レベルの HWND を確保し、winId() を実体化する。
// フォーカスを持たず、キーは親ウィンドウが受け取る。
//
// 重要: winId() を MpvBackend::attach() に渡した後は
//       このウィジェットを reparent しないこと
//       （HWND が変わり mpv の wid が stale になる）。
class VideoHostWidget : public QWidget {
    Q_OBJECT

public:
    explicit VideoHostWidget(QWidget* parent = nullptr);
};

}  // namespace yapcr::app
