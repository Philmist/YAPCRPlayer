#include "res_list_pane.h"

#include <QRegularExpression>
#include <QScrollBar>

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
        // レス間の空行（先頭または追記起点を除く）
        if (displayedCount_ > 0 || i > 0) {
            appendPlainText(QString{});
        }
        appendPlainText(formatRes(resList.at(i)));
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
}

}  // namespace yapcr::app
