#include "board_title_bar.h"

#include <QEnterEvent>
#include <QMouseEvent>
#include <QPainter>

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

void BoardTitleBar::leaveEvent(QEvent* event)
{
    QWidget::leaveEvent(event);
    emit left();
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
