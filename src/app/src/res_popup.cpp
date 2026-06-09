#include "res_popup.h"

#include <QGuiApplication>
#include <QPainter>
#include <QScreen>
#include <QWheelEvent>

namespace yapcr::app {

namespace {

QString formatResShort(const yapcr::bbs::ResInfo& r)
{
    QString header = r.number;
    if (!r.name.isEmpty())     { header += QLatin1Char(' ') + r.name; }
    if (!r.id.isEmpty())       { header += QStringLiteral(" [") + r.id + QLatin1Char(']'); }
    if (!r.datetime.isEmpty()) { header += QLatin1Char(' ') + r.datetime; }
    return header + QLatin1Char('\n') + r.message;
}

}  // namespace

ResPopup::ResPopup(QWidget* parent) : QWidget(parent)
{
    setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_ShowWithoutActivating);
    hide();
}

void ResPopup::showAt(const QList<yapcr::bbs::ResInfo>& resList, QPoint globalPos)
{
    if (resList.isEmpty()) {
        hide();
        return;
    }
    resList_    = resList;
    wheelIndex_ = 0;
    rebuildText();

    // サイズを fontMetrics で概算して確定する
    const QFontMetrics fm(font());
    const int textW = kMaxWidth - kPadding * 2;
    const QRect br   = fm.boundingRect(QRect(0, 0, textW, 0),
                                       Qt::TextWordWrap, displayText_);
    const int w = qMin(br.width()  + kPadding * 2 + 4, kMaxWidth);
    const int h = qMin(br.height() + kPadding * 2 + 4, kMaxHeight);
    setFixedSize(w, h);

    // 配置: globalPos の右下を基点に、画面外ならクランプ
    QScreen* scr = QGuiApplication::screenAt(globalPos);
    if (!scr) { scr = QGuiApplication::primaryScreen(); }
    const QRect avail = scr->availableGeometry();

    int x = globalPos.x() + 12;
    int y = globalPos.y() + 12;
    if (x + w > avail.right())  { x = globalPos.x() - w - 4; }
    if (y + h > avail.bottom()) { y = globalPos.y() - h - 4; }
    x = qMax(x, avail.left());
    y = qMax(y, avail.top());

    move(x, y);
    show();
}

void ResPopup::hidePopup()
{
    hide();
}

void ResPopup::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    p.fillRect(rect(), QColor(0, 0, 0, static_cast<int>(255 * kAlpha)));
    p.setPen(Qt::white);
    p.drawText(rect().adjusted(kPadding, kPadding, -kPadding, -kPadding),
               Qt::TextWordWrap, displayText_);
}

void ResPopup::wheelEvent(QWheelEvent* event)
{
    if (resList_.isEmpty()) { return; }
    const int delta = event->angleDelta().y();
    if (delta < 0) {
        wheelIndex_ = qMin(wheelIndex_ + 1, resList_.size() - 1);
    } else if (delta > 0) {
        wheelIndex_ = qMax(wheelIndex_ - 1, 0);
    }
    rebuildText();
    update();
    event->accept();
}

void ResPopup::rebuildText()
{
    if (resList_.isEmpty()) {
        displayText_.clear();
        return;
    }
    const int idx = qBound(0, wheelIndex_, resList_.size() - 1);
    const auto& r = resList_.at(idx);
    displayText_  = formatResShort(r);
    if (resList_.size() > 1) {
        displayText_ += QStringLiteral("\n[%1/%2]").arg(idx + 1).arg(resList_.size());
    }
}

}  // namespace yapcr::app
