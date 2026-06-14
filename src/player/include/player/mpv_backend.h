#pragma once

#include <QList>
#include <QObject>
#include <QString>
#include <QStringList>

// mpv_handle は C の前方宣言（mpv/client.h を公開ヘッダに含めない）。
struct mpv_handle;
// mpv_render_context は C の前方宣言（mpv/render.h を公開ヘッダに含めない）。
struct mpv_render_context;

namespace yapcr::player {

// libmpv のラッパ。Qt のイベントループに統合する。
//
// 使い方（Render API モード）:
//   Phase 1: attach() で mpv を初期化する。
//            attach() 完了後すぐ load() を呼んでよい（内部デコードを早期に開始する）。
//   Phase 2: VideoHostWidget::initializeGL() から initRenderContext() を呼ぶ。
//            以後 frameReady シグナル → update() → paintGL() で映像が出る。
//
// スレッド安全性:
//   mpv の内部スレッドは wakeupCallback / renderUpdateCallback のみを通じて
//   GUI スレッドと通信し、mpv API は常に GUI スレッドから呼ぶ。
class MpvBackend : public QObject {
    Q_OBJECT

public:
    explicit MpvBackend(QObject* parent = nullptr);
    ~MpvBackend() override;

    // Phase 1: mpv を初期化する。
    // wid/vo オプションは設定しない（Render API が vo=libmpv を自動選択する）。
    // 成功時 true を返す。
    bool attach();

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

    // Phase 2: OpenGL 描画コンテキストを初期化する。
    // VideoHostWidget::initializeGL() から呼ぶこと（GL コンテキストが current であること）。
    // getProcAddress: GL 関数ポインタリゾルバ（QOpenGLContext::getProcAddress ラッパ）
    // procAddrCtx:   getProcAddress に渡す不透明コンテキスト
    // 成功時 true を返す。
    bool initRenderContext(void* (*getProcAddress)(void*, const char*), void* procAddrCtx);

    // 現在のフレームを指定 FBO に描画する。
    // GL コンテキストが current な状態（paintGL 内）から呼ぶこと。
    // renderCtx が未生成（initRenderContext 前）の場合は no-op。
    void renderFrame(int fboId, int w, int h);

    // 描画コンテキストを解放する。
    // GL コンテキストが current な状態（VideoHostWidget デストラクタの makeCurrent 後）から呼ぶこと。
    void destroyRenderContext();

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
    void videoSizeChanged(int w, int h);

    // Render API: 新フレームが描画可能になった（mpv 内部スレッドから委譲）。
    // 受け取り側は update() を呼ぶ。
    void frameReady();

private:
    // mpv の wakeup callback（内部スレッドから呼ばれる）。
    // GUI スレッドの onWakeup() をキューイングするだけ。mpv API は呼ばない。
    static void wakeupCallback(void* ctx);

    // Render API の update callback（内部スレッドから呼ばれる）。
    // GUI スレッドの onRenderUpdate() をキューイングするだけ。mpv_render_* API は呼ばない。
    static void renderUpdateCallback(void* ctx);

private slots:
    // wakeupCallback の通知を受けて GUI スレッドで mpv イベントを排出する。
    void onWakeup();

    // renderUpdateCallback の通知を GUI スレッドで受ける。
    // mpv の update 通知は「再描画保留」ダーティフラグ方式で、mpv_render_context_update()
    // を呼んでフラグをクリアしない限り次のコールバックが発火しない。ここで毎回 update() を
    // 呼んでフラグをクリア（＝再アーム）し、MPV_RENDER_UPDATE_FRAME のときだけ frameReady を emit する。
    void onRenderUpdate();

private:
    mpv_handle*         mpv_{nullptr};
    mpv_render_context* renderCtx_{nullptr};

    // M4.0: setOption() で積まれた init 前オプション（attach() 内で適用）。
    struct PendingOption { QString name; QString value; };
    QList<PendingOption> pendingOptions_;

    // M4.0: dwidth/dheight の最終観測値（videoSizeChanged emit 用）。
    int lastVideoW_{0};
    int lastVideoH_{0};
};

}  // namespace yapcr::player
