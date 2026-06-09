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

// フォントメトリクスを使いテキストの表示サイズを概算する
QSize calcTextSize(const QFontMetrics& fm, const QString& text,
                   int maxWidth, int maxHeight, int padding)
{
    const int textW = maxWidth - padding * 2;
    const QRect br  = fm.boundingRect(QRect(0, 0, textW, 0),
                                      Qt::TextWordWrap, text);
    const int w = qMin(br.width()  + padding * 2 + 4, maxWidth);
    const int h = qMin(br.height() + padding * 2 + 4, maxHeight);
    return {w, h};
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
    recentMode_ = false;
    resList_    = resList;
    wheelIndex_ = 0;
    rebuildText();

    // サイズを fontMetrics で概算して確定する
    const QFontMetrics fm(font());
    const QSize sz = calcTextSize(fm, displayText_, kMaxWidth, kMaxHeight, kPadding);
    setFixedSize(sz);

    // 配置: globalPos の右下を基点に、画面外ならクランプ
    QScreen* scr = QGuiApplication::screenAt(globalPos);
    if (!scr) { scr = QGuiApplication::primaryScreen(); }
    const QRect avail = scr->availableGeometry();

    int x = globalPos.x() + 12;
    int y = globalPos.y() + 12;
    if (x + sz.width()  > avail.right())  { x = globalPos.x() - sz.width()  - 4; }
    if (y + sz.height() > avail.bottom()) { y = globalPos.y() - sz.height() - 4; }
    x = qMax(x, avail.left());
    y = qMax(y, avail.top());

    move(x, y);
    show();
}

void ResPopup::showRecent(const QList<yapcr::bbs::ResInfo>& all, QPoint anchorGlobal)
{
    if (all.isEmpty()) {
        hide();
        return;
    }
    recentMode_ = true;
    recentAll_  = all;
    windowEnd_  = all.size();  // 最初は末尾（最新レス）を基点にする
    rebuildText();

    // サイズ計算（複数レス分で kMaxHeight を大きめに使う）
    const QFontMetrics fm(font());
    const QSize sz = calcTextSize(fm, displayText_, kMaxWidth, kMaxHeight, kPadding);
    setFixedSize(sz);

    // 配置: タイトル帯（anchorGlobal）の直上に展開する
    QScreen* scr = QGuiApplication::screenAt(anchorGlobal);
    if (!scr) { scr = QGuiApplication::primaryScreen(); }
    const QRect avail = scr->availableGeometry();

    // 右寄り（カーソル位置基点）で上に展開
    int x = anchorGlobal.x();
    int y = anchorGlobal.y() - sz.height() - 4;

    // 画面端クランプ
    if (x + sz.width() > avail.right())  { x = avail.right()  - sz.width(); }
    if (y < avail.top())                 { y = anchorGlobal.y() + 4; }  // 上に出せない場合は下
    x = qMax(x, avail.left());

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
    const int delta = event->angleDelta().y();

    if (recentMode_) {
        // Recent モード: 上回し → 過去レスへ遡る（windowEnd_ を減らす）
        //               下回し → 最新レスへ戻る（windowEnd_ を増やす）
        if (delta > 0) {
            windowEnd_ = qMax(windowEnd_ - 1, qMin(kWindow, recentAll_.size()));
        } else if (delta < 0) {
            windowEnd_ = qMin(windowEnd_ + 1, recentAll_.size());
        }
    } else {
        // Single モード: 既存の動作を維持
        if (resList_.isEmpty()) { return; }
        if (delta < 0) {
            wheelIndex_ = qMin(wheelIndex_ + 1, resList_.size() - 1);
        } else if (delta > 0) {
            wheelIndex_ = qMax(wheelIndex_ - 1, 0);
        }
    }

    rebuildText();
    update();
    event->accept();
}

void ResPopup::leaveEvent(QEvent* event)
{
    QWidget::leaveEvent(event);
    // Recent モードではポップアップを離れたら自動的に閉じる
    if (recentMode_) {
        hide();
    }
}

void ResPopup::rebuildText()
{
    if (recentMode_) {
        // Recent モード: windowEnd_ を末尾として kWindow 件をスタック表示
        if (recentAll_.isEmpty()) {
            displayText_.clear();
            return;
        }
        const int end   = qBound(0, windowEnd_, recentAll_.size());
        const int begin = qMax(0, end - kWindow);
        QStringList lines;
        for (int i = begin; i < end; ++i) {
            lines.append(formatResShort(recentAll_.at(i)));
        }
        // フッター: 表示範囲 / 全体
        const int dispFirst = begin + 1;
        const int dispLast  = end;
        lines.append(QStringLiteral("[%1-%2 / %3]")
                         .arg(dispFirst).arg(dispLast).arg(recentAll_.size()));
        displayText_ = lines.join(QLatin1Char('\n'));
    } else {
        // Single モード: 既存の動作
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
}

}  // namespace yapcr::app
