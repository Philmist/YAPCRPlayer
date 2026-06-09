#pragma once

#include <QWidget>

class QPlainTextEdit;
class QPushButton;

namespace yapcr::app {

// 下部レス入力バー（M3.6）
//
// 本文 QPlainTextEdit（複数行・改行入力可）＋「送信」QPushButton。
// Enter=改行（QPlainTextEdit 既定）、Ctrl+Enter=送信。
// 送信ボタンクリックでも送信できる。
class ResInputBar : public QWidget {
    Q_OBJECT

public:
    explicit ResInputBar(QWidget* parent = nullptr);

    // 入力欄をクリアする（書き込み成功後に呼ぶ）。
    void clearInput();

    // 入力欄・送信ボタンを有効/無効にする（書き込み中は false で無効化）。
    void setInputEnabled(bool enabled);

signals:
    // 送信ボタンまたは Ctrl+Enter が押されたとき emit される。
    // message: 入力欄の全テキスト（改行を含む場合あり）。
    // 空文字列の場合は emit しない。
    void postRequested(const QString& message);

private slots:
    void onSendClicked();

private:
    QPlainTextEdit* edit_{nullptr};
    QPushButton*    sendBtn_{nullptr};
};

}  // namespace yapcr::app
