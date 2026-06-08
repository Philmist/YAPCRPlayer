#include "video_host_widget.h"

#include <QPalette>

namespace yapcr::app {

VideoHostWidget::VideoHostWidget(QWidget* parent) : QWidget(parent) {
    // WA_NativeWindow: OS レベルの HWND を確保（winId() の実体化に必須）。
    // mpv はこの HWND を親として子ウィンドウを作成して映像を描画する。
    setAttribute(Qt::WA_NativeWindow);

    // フォーカスを奪わない（キー入力は MainWindow が受け取る）
    setFocusPolicy(Qt::NoFocus);

    // 映像表示前の背景を黒にする
    QPalette pal = palette();
    pal.setColor(QPalette::Window, Qt::black);
    setPalette(pal);
    setAutoFillBackground(true);
}

}  // namespace yapcr::app
