#pragma once

#include <QList>
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
//   2. 必要なら setOption() でオプションを積む（attach() 前専用）。
//   3. attach(videoWidget->winId()) で mpv を初期化してアタッチする。
//   4. load(url) で再生を開始する。
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

    // ---- M4.0: プロパティ/オプション API ----------------------------------

    // init 前専用オプションを積む（attach() 呼び出し時に mpv_initialize 前へ適用される）。
    // attach() 完了後（init 済み）の呼び出しは no-op。
    void setOption(const QString& name, const QString& value);

    // 実行時プロパティを文字列で設定する（init 後専用）。
    void setProperty(const QString& name, const QString& value);

    // 実行時プロパティを bool フラグで設定する（MPV_FORMAT_FLAG）。
    void setPropertyFlag(const QString& name, bool on);

    // 実行時プロパティを double で設定する（MPV_FORMAT_DOUBLE）。
    void setPropertyDouble(const QString& name, double v);

    // プロパティを double で同期取得する。失敗時は 0.0 を返す。
    double getPropertyDouble(const QString& name) const;

    // プロパティを文字列で同期取得する。失敗時は空文字列を返す。
    QString getPropertyString(const QString& name) const;

    // -----------------------------------------------------------------------

signals:
    // ファイルが読み込まれ再生が始まった。
    void fileLoaded();

    // 再生が終わった。reason は mpv_end_file_reason の値:
    //   0=EOF, 1=stop, 2=quit, 3=error, 4=redirect, 5=restart
    void endFile(int reason);

    // mpv のログメッセージ。
    void logMessage(const QString& prefix, const QString& level, const QString& text);

    // core-idle プロパティが変化した（idle=true: 映像供給停止中）。
    void coreIdleChanged(bool idle);

    // demuxer-cache-time プロパティが変化した（キャッシュ内の総秒数）。
    void cacheTimeChanged(double seconds);

    // M4.0: dwidth/dheight（アスペクト適用後の表示寸法）が変化した。
    // 両方が正値になったとき、またはいずれかが変化したときに emit する。
    // サブプラン横断決定 3: ズーム%・サイズプリセットの基準値として使う。
    void videoSizeChanged(int w, int h);

private:
    // mpv の wakeup callback（内部スレッドから呼ばれる）。
    // GUI スレッドの onWakeup() をキューイングするだけ。mpv API は呼ばない。
    static void wakeupCallback(void* ctx);

private slots:
    // wakeupCallback の通知を受けて GUI スレッドで mpv イベントを排出する。
    void onWakeup();

private:
    mpv_handle* mpv_{nullptr};

    // M4.0: setOption() で積まれた init 前オプション（attach() 内で適用）。
    struct PendingOption { QString name; QString value; };
    QList<PendingOption> pendingOptions_;

    // M4.0: dwidth/dheight の最終観測値（videoSizeChanged emit 用）。
    int lastVideoW_{0};
    int lastVideoH_{0};
};

}  // namespace yapcr::player
