#include "board_title_bar.h"

#include <QContextMenuEvent>
#include <QEnterEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QWheelEvent>

namespace yapcr::app {

BoardTitleBar::BoardTitleBar(QWidget* parent) : QWidget(parent)
{
    setFixedHeight(kHeight);
    setMouseTracking(true);
    // 初期テキスト（BBS 未取得時）
    text_ = QStringLiteral("[ — ]");
}

void BoardTitleBar::setInfo(const QString& title, int count)
{
    text_ = QStringLiteral("[ %1 ]( %2 )").arg(title).arg(count);
    update();
}

void BoardTitleBar::enterEvent(QEnterEvent* event)
{
    QWidget::enterEvent(event);
    emit hovered(event->globalPosition().toPoint());
}

void BoardTitleBar::mouseMoveEvent(QMouseEvent* event)
{
    QWidget::mouseMoveEvent(event);
    emit hovered(event->globalPosition().toPoint());
}

void BoardTitleBar::mouseDoubleClickEvent(QMouseEvent* event)
{
    // 映像外でも帯のダブルクリックで全画面トグルを維持する（catch-all 撤去の代替）。
    // accept して親（MainWindow）への伝播を止める。
    if (event->button() == Qt::LeftButton) {
        emit fullscreenToggleRequested();
    }
    event->accept();
}

void BoardTitleBar::leaveEvent(QEvent* event)
{
    QWidget::leaveEvent(event);
    emit left();
}

void BoardTitleBar::wheelEvent(QWheelEvent* event)
{
    // 帯上のホイールをポップアップの遡行へ転送する（カーソルは帯に置いたまま）
    emit scrolled(event->angleDelta().y());
    event->accept();
}

void BoardTitleBar::contextMenuEvent(QContextMenuEvent* event)
{
    emit contextMenuRequested(event->globalPos());
    event->accept();
}

void BoardTitleBar::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    // 暗い帯（PCRPlayer の下部タイトル帯に合わせた配色）
    p.fillRect(rect(), QColor(0x1a, 0x1a, 0x1a));
    p.setPen(Qt::white);
    p.drawText(rect(), Qt::AlignCenter, text_);
}

}  // namespace yapcr::app
