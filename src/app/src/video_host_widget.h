#pragma once

#include <QOpenGLFunctions>
#include <QOpenGLWidget>
#include <QPoint>

namespace yapcr::player {
class MpvBackend;
}

namespace yapcr::app {

// libmpv Render API による映像描画ウィジェット。
//
// QOpenGLWidget のサブクラスとして mpv の OpenGL レンダラと統合する。
// フォーカスを持たず、キーは親ウィンドウが受け取る。
//
// 初期化は 2 フェーズ:
//   Phase 1: MpvBackend::attach()（MainWindow::attachMpv() 内）
//   Phase 2: initializeGL()（ウィジェットが初めて表示されるときに Qt が自動で呼ぶ）
//
// マウスイベントはシグナルで MainWindow に転送する（旧 nativeEvent の代替）。
class VideoHostWidget : public QOpenGLWidget, protected QOpenGLFunctions {
    Q_OBJECT

public:
    // mpv: MpvBackend のポインタ。このウィジェットより長生きすること（MainWindow が保証）。
    explicit VideoHostWidget(yapcr::player::MpvBackend* mpv, QWidget* parent = nullptr);
    ~VideoHostWidget() override;

signals:
    // initializeGL() で mpv レンダーコンテキストの初期化が完了したとき発行される。
    // このシグナルの前に loadfile を送ると mpv が独自ウィンドウを開くため、
    // MainWindow は pending media の再生開始をこのシグナルまで遅延させる。
    void renderContextReady();
    // 映像領域ドラッグでウィンドウを移動させる（グローバル座標の差分）。
    void windowDragRequested(QPoint globalDelta);
    // ダブルクリックで全画面トグルを要求する。
    void fullscreenToggleRequested();
    // 右クリックでコンテキストメニューを要求する（グローバル座標）。
    void contextMenuRequested(QPoint globalPos);
    // 左クリックで親ウィンドウへフォーカスを戻すよう要求する。
    void focusRequested();

protected:
    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int w, int h) override;

    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    void contextMenuEvent(QContextMenuEvent* event) override;

private:
    yapcr::player::MpvBackend* mpv_{nullptr};

    // ドラッグ追従用（mousePressEvent で記録、mouseMoveEvent で差分を emit）
    bool   dragging_{false};
    QPoint dragLastGlobal_;
};

}  // namespace yapcr::app
