#pragma once

#include <QObject>
#include <QString>
#include <QStringList>

// mpv_handle は C の前方宣言（mpv/client.h を公開ヘッダに含めない）。
struct mpv_handle;

namespace yapcr::player {

// libmpv のラッパ。Qt のイベントループに統合する。
//
// 使い方:
//   1. MainWindow を show() して VideoHostWidget の winId() を実体化する。
//   2. attach(videoWidget->winId()) で mpv を初期化してアタッチする。
//   3. load(url) で再生を開始する。
//
// スレッド安全性:
//   mpv の内部スレッドは wakeupCallback のみを通じて GUI スレッドと通信し、
//   mpv API は常に GUI スレッドから呼ぶ。
class MpvBackend : public QObject {
    Q_OBJECT

public:
    explicit MpvBackend(QObject* parent = nullptr);
    ~MpvBackend() override;

    // 映像を埋め込むウィジェットの winId() を渡して mpv を初期化する。
    // 型は quintptr（Qt6::Core 依存のみ; WId は Qt::Gui なので使わない）。
    // 呼び出し前にウィンドウを show() して winId() を実体化しておくこと。
    // attach 後にウィジェットを reparent しないこと（HWND が変わり wid が stale になる）。
    // 成功時 true を返す。
    bool attach(quintptr wid);

    // url/ファイルパスを再生する（mpv の "loadfile" コマンド）。
    void load(const QString& url);

    // 任意の mpv コマンドを発行する。
    void command(const QStringList& args);

signals:
    // ファイルが読み込まれ再生が始まった。
    void fileLoaded();

    // 再生が終わった。reason は mpv_end_file_reason の値:
    //   0=EOF, 1=stop, 2=quit, 3=error, 4=redirect, 5=restart
    void endFile(int reason);

    // mpv のログメッセージ。
    void logMessage(const QString& prefix, const QString& level, const QString& text);

private:
    // mpv の wakeup callback（内部スレッドから呼ばれる）。
    // GUI スレッドの onWakeup() をキューイングするだけ。mpv API は呼ばない。
    static void wakeupCallback(void* ctx);

private slots:
    // wakeupCallback の通知を受けて GUI スレッドで mpv イベントを排出する。
    void onWakeup();

private:
    mpv_handle* mpv_{nullptr};
};

}  // namespace yapcr::player
