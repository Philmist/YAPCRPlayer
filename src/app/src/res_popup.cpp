#include "res_popup.h"

#include "res_html.h"

#include <QGuiApplication>
#include <QPainter>
#include <QScreen>
#include <QWheelEvent>
#include <QtMath>

namespace yapcr::app {

ResPopup::ResPopup(QWidget* parent) : QWidget(parent)
{
    setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_ShowWithoutActivating);
    doc_.setDefaultStyleSheet(QStringLiteral("a { text-decoration: none; }"));
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
    setFixedSize(computeSize());

    // 配置: globalPos の右下を基点に、画面外ならクランプ
    QScreen* scr = QGuiApplication::screenAt(globalPos);
    if (!scr) { scr = QGuiApplication::primaryScreen(); }
    const QRect avail = scr->availableGeometry();

    int x = globalPos.x() + 12;
    int y = globalPos.y() + 12;
    if (x + width()  > avail.right())  { x = globalPos.x() - width()  - 4; }
    if (y + height() > avail.bottom()) { y = globalPos.y() - height() - 4; }
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
    // 既に Recent モードで表示中なら、配置と遡行位置は維持する。
    // （タイトル帯上でのマウス移動ごとに再配置・最新リセットすると、
    //   ポップアップがカーソルに追随して動き、遡行位置も毎回戻ってしまうため）
    const bool wasVisible = recentMode_ && isVisible();

    recentMode_ = true;
    recentAll_  = all;
    if (!wasVisible) {
        windowEnd_ = all.size();  // 初回のみ末尾（最新レス）を基点にする
    } else {
        // データ更新でサイズが変わっても遡行位置を範囲内に保つ
        windowEnd_ = qBound(qMin(kWindow, all.size()), windowEnd_, all.size());
    }
    rebuildText();
    setFixedSize(computeSize());

    // 配置は初回表示時のみ確定する（以後はカーソル追随で動かさない）
    if (!wasVisible) {
        // 配置: タイトル帯（anchorGlobal）の直上に展開する
        QScreen* scr = QGuiApplication::screenAt(anchorGlobal);
        if (!scr) { scr = QGuiApplication::primaryScreen(); }
        const QRect avail = scr->availableGeometry();

        // 右寄り（カーソル位置基点）で上に展開
        int x = anchorGlobal.x();
        int y = anchorGlobal.y() - height() - 4;

        // 画面端クランプ
        if (x + width() > avail.right())  { x = avail.right() - width(); }
        if (y < avail.top())              { y = anchorGlobal.y() + 4; }  // 上に出せない場合は下
        x = qMax(x, avail.left());

        move(x, y);
        show();
    }
}

void ResPopup::scrollRecent(int delta)
{
    if (!recentMode_ || !isVisible() || recentAll_.isEmpty()) { return; }

    // 上回し → 過去レスへ遡る（windowEnd_ を減らす）
    // 下回し → 最新レスへ戻る（windowEnd_ を増やす）
    if (delta > 0) {
        windowEnd_ = qMax(windowEnd_ - 1, qMin(kWindow, recentAll_.size()));
    } else if (delta < 0) {
        windowEnd_ = qMin(windowEnd_ + 1, recentAll_.size());
    }
    rebuildText();
    update();
}

void ResPopup::hidePopup()
{
    hide();
}

QSize ResPopup::computeSize()
{
    doc_.setDefaultFont(font());
    // まず最大幅で割り付け、必要十分な幅(idealWidth)を求めてから再割り付けして高さを確定する
    const qreal maxContentW = kMaxWidth - kPadding * 2;
    doc_.setTextWidth(maxContentW);
    const qreal contentW = qMin(doc_.idealWidth(), maxContentW);
    doc_.setTextWidth(contentW);
    const QSizeF s = doc_.size();

    const int w = qMin(qCeil(s.width())  + kPadding * 2, kMaxWidth);
    const int h = qMin(qCeil(s.height()) + kPadding * 2, kMaxHeight);
    return {w, h};
}

void ResPopup::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    // ライト半透明背景（一覧ペインと同パレットの色が読めるように）
    p.fillRect(rect(), QColor(250, 250, 250, static_cast<int>(255 * kAlpha)));
    p.setPen(QColor(170, 170, 170));
    p.drawRect(rect().adjusted(0, 0, -1, -1));

    p.translate(kPadding, kPadding);
    const QRectF clip(0, 0, width() - kPadding * 2, height() - kPadding * 2);
    doc_.drawContents(&p, clip);
}

void ResPopup::wheelEvent(QWheelEvent* event)
{
    const int delta = event->angleDelta().y();

    if (recentMode_) {
        // Recent モード: ポップアップ自身の上でのホイールも遡行に使う
        scrollRecent(delta);
    } else {
        // Single モード: 既存の動作を維持
        if (resList_.isEmpty()) { return; }
        if (delta < 0) {
            wheelIndex_ = qMin(wheelIndex_ + 1, resList_.size() - 1);
        } else if (delta > 0) {
            wheelIndex_ = qMax(wheelIndex_ - 1, 0);
        }
        rebuildText();
        update();
    }
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
    QString html;

    if (recentMode_) {
        // Recent モード: windowEnd_ を末尾として kWindow 件をスタック表示
        if (recentAll_.isEmpty()) {
            doc_.clear();
            return;
        }
        const int end   = qBound(0, windowEnd_, recentAll_.size());
        const int begin = qMax(0, end - kWindow);
        QStringList frags;
        for (int i = begin; i < end; ++i) {
            frags.append(reshtml::resToHtml(recentAll_.at(i), /*withAnchorLinks=*/false));
        }
        html = frags.join(QStringLiteral("<hr>"));
        // フッタ: 表示範囲 / 全体
        html += QStringLiteral("<hr><div style=\"color:%1;\">[%2-%3 / %4]</div>")
                    .arg(reshtml::kTimeColor,
                         QString::number(begin + 1),
                         QString::number(end),
                         QString::number(recentAll_.size()));
    } else {
        // Single モード: 既存の動作
        if (resList_.isEmpty()) {
            doc_.clear();
            return;
        }
        const int idx = qBound(0, wheelIndex_, resList_.size() - 1);
        html = reshtml::resToHtml(resList_.at(idx), /*withAnchorLinks=*/false);
        if (resList_.size() > 1) {
            html += QStringLiteral("<hr><div style=\"color:%1;\">[%2/%3]</div>")
                        .arg(reshtml::kTimeColor,
                             QString::number(idx + 1),
                             QString::number(resList_.size()));
        }
    }

    doc_.setHtml(html);
}

}  // namespace yapcr::app
