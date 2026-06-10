#include "res_input_bar.h"

#include <QFontMetrics>
#include <QHBoxLayout>
#include <QKeySequence>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QShortcut>
#include <QSizePolicy>

namespace yapcr::app {

ResInputBar::ResInputBar(QWidget* parent) : QWidget(parent)
{
    edit_    = new QPlainTextEdit(this);
    sendBtn_ = new QPushButton(tr("送信(&S)"), this);

    // ClickFocus: ユーザーが明示的にクリックしたときだけフォーカスを取る。
    // StrongFocus（既定）のままだと inputDock_->show() 時に自動フォーカスが移り
    // MainWindow::keyPressEvent に F/S/Esc が届かなくなる。
    edit_->setFocusPolicy(Qt::ClickFocus);

    // 入力欄は低背（2行）。PCRPlayer 参考: 書き込みバーはスリム。
    edit_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    const QFontMetrics fm(edit_->font());
    const int lineH = fm.lineSpacing();
    edit_->setFixedHeight(lineH * 2 + 8);

    sendBtn_->setFocusPolicy(Qt::TabFocus);
    sendBtn_->setFixedWidth(80);

    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(4, 4, 4, 4);
    layout->addWidget(edit_);
    layout->addWidget(sendBtn_);

    connect(sendBtn_, &QPushButton::clicked, this, &ResInputBar::onSendClicked);

    // Ctrl+Enter でも送信（Enter 単体は改行として使いたいため Ctrl 修飾）
    auto* shortcut = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_Return), edit_);
    connect(shortcut, &QShortcut::activated, this, &ResInputBar::onSendClicked);
}

void ResInputBar::clearInput()
{
    edit_->clear();
}

void ResInputBar::setInputEnabled(bool enabled)
{
    edit_->setEnabled(enabled);
    sendBtn_->setEnabled(enabled);
}

void ResInputBar::onSendClicked()
{
    const QString msg = edit_->toPlainText().trimmed();
    if (msg.isEmpty()) { return; }
    emit postRequested(msg);
}

}  // namespace yapcr::app
