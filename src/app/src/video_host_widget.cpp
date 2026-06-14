#include "video_host_widget.h"
#include "player/mpv_backend.h"

#include <QContextMenuEvent>
#include <QMouseEvent>
#include <QOpenGLContext>
#include <QOpenGLFunctions>

namespace yapcr::app {

VideoHostWidget::VideoHostWidget(yapcr::player::MpvBackend* mpv, QWidget* parent)
    : QOpenGLWidget(parent), mpv_(mpv)
{
    // フォーカスを奪わない（キー入力は MainWindow が受け取る）
    setFocusPolicy(Qt::NoFocus);
}

VideoHostWidget::~VideoHostWidget()
{
    // GL コンテキストを current にしてから描画コンテキストを解放する。
    // MpvBackend（mpv_）はこのウィジェットより長生きすること（MainWindow が保証）。
    makeCurrent();
    mpv_->destroyRenderContext();
    doneCurrent();
}

// Phase 2: GL コンテキストが確立された直後に Qt が自動で呼ぶ。
void VideoHostWidget::initializeGL()
{
    // QOpenGLFunctions の glClear / glClearColor 等を有効化する（Qt イディオム）。
    initializeOpenGLFunctions();

    QOpenGLContext* ctx = QOpenGLContext::currentContext();
    if (!ctx) {
        qWarning() << "[VideoHostWidget] initializeGL: GL コンテキストが取得できない";
        return;
    }

    // getProcAddress ラッパ。ctx を void* として受け取り QOpenGLContext 経由で解決する。
    auto getProcAddr = [](void* ctxPtr, const char* name) -> void* {
        return reinterpret_cast<void*>(
            static_cast<QOpenGLContext*>(ctxPtr)->getProcAddress(name));
    };

    if (!mpv_->initRenderContext(getProcAddr, ctx)) {
        qWarning() << "[VideoHostWidget] initRenderContext 失敗";
        return;
    }

    // 新フレーム通知 → update() でリペイントを要求する（QueuedConnection で GUI スレッドに届く）。
    connect(mpv_, &yapcr::player::MpvBackend::frameReady,
            this, qOverload<>(&QWidget::update),
            Qt::QueuedConnection);

    // レンダーコンテキスト確立を MainWindow へ通知する。
    // この通知を受けるまで loadfile を送ると mpv が独自ウィンドウにフォールバックする。
    emit renderContextReady();
}

void VideoHostWidget::paintGL()
{
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // devicePixelRatio() で論理ピクセルを物理ピクセルに変換する。
    // フレームバッファオブジェクトは QOpenGLWidget が管理するもの（0 ではない）。
    const qreal dpr = devicePixelRatio();
    mpv_->renderFrame(
        static_cast<int>(defaultFramebufferObject()),
        static_cast<int>(width()  * dpr),
        static_cast<int>(height() * dpr));
}

void VideoHostWidget::resizeGL(int /*w*/, int /*h*/)
{
    // paintGL が毎回 FBO サイズを渡すため、個別の対応は不要。
}

// ---- マウスイベント（旧 nativeEvent + ドラッグポーリングの置き換え） --------

void VideoHostWidget::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        emit focusRequested();      // 旧 nativeEvent での setFocus(Qt::MouseFocusReason) 相当
        dragging_       = true;
        dragLastGlobal_ = event->globalPosition().toPoint();
    }
    QOpenGLWidget::mousePressEvent(event);
}

void VideoHostWidget::mouseMoveEvent(QMouseEvent* event)
{
    if (dragging_ && (event->buttons() & Qt::LeftButton)) {
        const QPoint current = event->globalPosition().toPoint();
        const QPoint delta   = current - dragLastGlobal_;
        dragLastGlobal_      = current;
        if (!delta.isNull()) {
            emit windowDragRequested(delta);
        }
    }
    QOpenGLWidget::mouseMoveEvent(event);
}

void VideoHostWidget::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        dragging_ = false;
    }
    QOpenGLWidget::mouseReleaseEvent(event);
}

void VideoHostWidget::mouseDoubleClickEvent(QMouseEvent* event)
{
    // accept して親への伝播を止める（contextMenuEvent と対称。二重トグルの再発防止）。
    if (event->button() == Qt::LeftButton) {
        emit fullscreenToggleRequested();
    }
    event->accept();
}

void VideoHostWidget::contextMenuEvent(QContextMenuEvent* event)
{
    emit contextMenuRequested(event->globalPos());
    event->accept();
}

}  // namespace yapcr::app
