#include "res_list_pane.h"

#include <QMouseEvent>
#include <QRegularExpression>
#include <QScrollBar>
#include <QTextBlock>

namespace yapcr::app {

namespace {

// <br> → 改行、残タグ除去の簡易 HTML ストリッパ（M3.6 用 plain text 表示）
// M3.7 で QTextDocument ベースのリッチ表示に置き換えてよい。
static QString stripHtml(const QString& html)
{
    QString s = html;
    // <br>/<BR> を改行に変換
    static const QRegularExpression rxBr(
        QStringLiteral(R"(<br\s*/?>)"),
        QRegularExpression::CaseInsensitiveOption);
    s.replace(rxBr, QStringLiteral("\n"));
    // 残タグを除去
    static const QRegularExpression rxTag(QStringLiteral(R"(<[^>]+>)"));
    s.remove(rxTag);
    // 最低限の HTML エンティティを戻す
    s.replace(QStringLiteral("&gt;"),   QStringLiteral(">"));
    s.replace(QStringLiteral("&lt;"),   QStringLiteral("<"));
    s.replace(QStringLiteral("&amp;"),  QStringLiteral("&"));
    s.replace(QStringLiteral("&nbsp;"), QStringLiteral(" "));
    s.replace(QStringLiteral("&quot;"), QStringLiteral("\""));
    return s;
}

// ResInfo → 表示テキスト（1レス分）
// ヘッダ行: "番号 名前 [ID] 日時"
// 本文行: HTML ストリップ済み
static QString formatRes(const yapcr::bbs::ResInfo& r)
{
    QString header = r.number;
    if (!r.name.isEmpty())     { header += QLatin1Char(' ') + stripHtml(r.name); }
    if (!r.id.isEmpty())       { header += QStringLiteral(" [") + r.id + QLatin1Char(']'); }
    if (!r.datetime.isEmpty()) { header += QLatin1Char(' ') + r.datetime; }
    const QString body = stripHtml(r.message);
    return header + QLatin1Char('\n') + body;
}

}  // namespace

ResListPane::ResListPane(QWidget* parent) : QPlainTextEdit(parent)
{
    setReadOnly(true);
    setLineWrapMode(QPlainTextEdit::WidgetWidth);
    // 最大ブロック数の制限を外す（QPlainTextEdit デフォルトの 100 は少なすぎる）
    setMaximumBlockCount(0);

    popup_ = new ResPopup(this);
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

    for (int i = displayedCount_; i < resList.size(); ++i) {
        // レス間の空行（先頭または追記起点を除く）— userState はデフォルト(-1) のまま
        if (displayedCount_ > 0 || i > 0) {
            appendPlainText(QString{});
        }

        // 追記前のブロック数を記録してからテキストを追記し、
        // 追記後の新規ブロック群にレス番号 (i+1) を埋め込む（M3.7 ホバー判定用）
        const int blkBefore = document()->blockCount();
        appendPlainText(formatRes(resList.at(i)));
        const int blkAfter  = document()->blockCount();
        const int resNum    = i + 1;
        for (int b = blkBefore; b < blkAfter; ++b) {
            document()->findBlockByNumber(b).setUserState(resNum);
        }
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
    lastHoveredRes_ = -1;
    popup_->hidePopup();
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
    QPlainTextEdit::mouseMoveEvent(event);
    const QPoint vpos = event->pos();

    // 1) ヘッダ行ホバー → byRef（返信群）
    const int headerRes = resNumberAt(vpos);
    if (headerRes > 0 && headerRes != lastHoveredRes_) {
        lastHoveredRes_ = headerRes;
        if (byRefProvider_) {
            popup_->showAt(byRefProvider_(headerRes),
                           event->globalPosition().toPoint());
        }
        return;
    }

    // 2) 本文 >>N ホバー → byRange（指定レス単体）
    const int anchorN = anchorTargetAt(vpos);
    if (anchorN > 0 && anchorN != lastHoveredRes_) {
        lastHoveredRes_ = anchorN;
        if (byRangeProvider_) {
            popup_->showAt(byRangeProvider_(yapcr::bbs::Range{anchorN, anchorN}),
                           event->globalPosition().toPoint());
        }
        return;
    }

    // 3) どちらでもない → ポップアップを閉じる
    if (headerRes < 0 && anchorN < 0) {
        lastHoveredRes_ = -1;
        popup_->hidePopup();
    }
}

void ResListPane::leaveEvent(QEvent* event)
{
    QPlainTextEdit::leaveEvent(event);
    lastHoveredRes_ = -1;
    popup_->hidePopup();
}

int ResListPane::resNumberAt(QPoint vpos) const
{
    const QTextBlock blk = cursorForPosition(vpos).block();
    const int resNum = blk.userState();
    if (resNum <= 0) { return -1; }

    // 先頭ブロック（前のブロックが別の resNum）ならヘッダ行と判定
    const QTextBlock prev = blk.previous();
    return (!prev.isValid() || prev.userState() != resNum) ? resNum : -1;
}

int ResListPane::anchorTargetAt(QPoint vpos) const
{
    const QTextBlock blk = cursorForPosition(vpos).block();
    if (blk.userState() <= 0) { return -1; }

    // 先頭ブロックはヘッダ行なので除外
    const QTextBlock prev = blk.previous();
    if (!prev.isValid() || prev.userState() != blk.userState()) { return -1; }

    // 表示テキストは stripHtml で &gt; → > に変換済みなので ASCII > に当てる
    static const QRegularExpression rxAnchor(QStringLiteral(R"(>{1,2}(\d+))"));
    const QString line    = blk.text();
    const int     posInBlk = cursorForPosition(vpos).positionInBlock();

    QRegularExpressionMatchIterator it = rxAnchor.globalMatch(line);
    while (it.hasNext()) {
        const QRegularExpressionMatch m = it.next();
        if (m.capturedStart() <= posInBlk && posInBlk <= m.capturedEnd()) {
            bool ok = false;
            const int n = m.captured(1).toInt(&ok);
            return ok ? n : -1;
        }
    }
    return -1;
}

}  // namespace yapcr::app
