#include "res_list_pane.h"

#include "res_html.h"

#include <QMouseEvent>
#include <QScrollBar>
#include <QTextCursor>
#include <QTextDocument>

namespace yapcr::app {

ResListPane::ResListPane(QWidget* parent) : QTextBrowser(parent)
{
    setReadOnly(true);
    setLineWrapMode(QTextBrowser::WidgetWidth);
    // アンカーは自前のホバー処理で扱うため、QTextBrowser の自動ナビゲーションは無効化する。
    setOpenLinks(false);
    setOpenExternalLinks(false);
    // アンカーの下線を消す（色はインラインスタイルで個別指定）。
    document()->setDefaultStyleSheet(QStringLiteral("a { text-decoration: none; }"));

    popup_ = new ResPopup(this);
    setMouseTracking(true);
    viewport()->setMouseTracking(true);
}

void ResListPane::setThreadTitle(const QString& title)
{
    threadTitle_ = title;
}

void ResListPane::appendResList(const QList<yapcr::bbs::ResInfo>& resList)
{
    if (resList.size() <= displayedCount_) {
        return;  // 新着なし
    }

    // ユーザーが末尾付近にいるか判定（追記後に自動スクロールするため）
    const bool atBottom =
        verticalScrollBar()->value() >= verticalScrollBar()->maximum() - 4;

    QTextCursor cur(document());
    cur.movePosition(QTextCursor::End);

    for (int i = displayedCount_; i < resList.size(); ++i) {
        // レス間の区切り（先頭レス以外に水平線）
        if (i > 0) {
            cur.insertHtml(QStringLiteral("<hr>"));
        }
        cur.insertHtml(reshtml::resToHtml(resList.at(i), /*withAnchorLinks=*/true));
    }
    displayedCount_ = resList.size();

    if (atBottom) {
        verticalScrollBar()->setValue(verticalScrollBar()->maximum());
    }
}

void ResListPane::clearRes()
{
    clear();
    displayedCount_ = 0;
    threadTitle_.clear();
    lastHoveredHref_.clear();
    popup_->hidePopup();
}

// M6: ThreadScrollNext/Prev — 1 ページ分スクロール
void ResListPane::scrollNext()
{
    QScrollBar* sb = verticalScrollBar();
    sb->setValue(sb->value() + sb->pageStep());
}

void ResListPane::scrollPrev()
{
    QScrollBar* sb = verticalScrollBar();
    sb->setValue(sb->value() - sb->pageStep());
}

void ResListPane::setByRefProvider(ResQueryFn fn)
{
    byRefProvider_ = std::move(fn);
}

void ResListPane::setByRangeProvider(RangeQueryFn fn)
{
    byRangeProvider_ = std::move(fn);
}

void ResListPane::mouseMoveEvent(QMouseEvent* event)
{
    QTextBrowser::mouseMoveEvent(event);

    const QString href = anchorAt(event->pos());
    if (href == lastHoveredHref_) {
        return;  // 同一アンカー上の移動は無視（再表示の連打抑止）
    }
    lastHoveredHref_ = href;

    if (href.isEmpty()) {
        popup_->hidePopup();
        return;
    }

    const QPoint gpos = event->globalPosition().toPoint();

    // ヘッダ行のレス番: "ref:N" → byRef（N を参照する返信群）
    if (href.startsWith(QLatin1String("ref:"))) {
        bool ok = false;
        const int n = href.mid(4).toInt(&ok);
        if (ok && byRefProvider_) {
            popup_->showAt(byRefProvider_(n), gpos);
        }
        return;
    }

    // 本文アンカー: "range:a-b" → byRange（指定レス範囲）
    if (href.startsWith(QLatin1String("range:"))) {
        const QString spec = href.mid(6);
        const int dash = spec.indexOf(QLatin1Char('-'));
        bool ok1 = false, ok2 = false;
        const int a = (dash < 0 ? spec : spec.left(dash)).toInt(&ok1);
        const int b = (dash < 0 ? spec : spec.mid(dash + 1)).toInt(&ok2);
        if (ok1 && ok2 && byRangeProvider_) {
            popup_->showAt(byRangeProvider_(yapcr::bbs::Range{a, b}), gpos);
        }
        return;
    }

    popup_->hidePopup();
}

void ResListPane::leaveEvent(QEvent* event)
{
    QTextBrowser::leaveEvent(event);
    lastHoveredHref_.clear();
    popup_->hidePopup();
}

}  // namespace yapcr::app
