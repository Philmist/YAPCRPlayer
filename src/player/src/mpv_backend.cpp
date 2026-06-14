#include "player/mpv_backend.h"

#include <mpv/client.h>
#include <mpv/render_gl.h>   // mpv_opengl_init_params / mpv_opengl_fbo / render API 全般

#include <QMetaObject>
#include <stdexcept>
#include <utility>
#include <vector>

namespace yapcr::player {

namespace {

void checkMpvError(int status) {
    if (status < 0) {
        throw std::runtime_error(mpv_error_string(status));
    }
}

}  // namespace

MpvBackend::MpvBackend(QObject* parent) : QObject(parent) {}

MpvBackend::~MpvBackend() {
    if (mpv_) {
        // wakeup callback を無効化して新たな invokeMethod の投入を止める。
        mpv_set_wakeup_callback(mpv_, nullptr, nullptr);

        // renderCtx が残存している場合（正規ルートはウィジェット側での解放）は
        // GL コンテキスト無しで呼ぶことになるため、update callback のみ無効化して
        // mpv 側に解放を委ねる（VO 終了時に内部で解放される）。
        // 通常はウィジェット~VideoHostWidget() で destroyRenderContext() が呼ばれており
        // renderCtx_ == nullptr のはずである。
        renderCtx_ = nullptr;  // dangling 防止

        mpv_handle* h = std::exchange(mpv_, nullptr);
        mpv_terminate_destroy(h);
    }
}

bool MpvBackend::attach() {
    if (mpv_) { return false; }  // 二重 attach は不可

    mpv_handle* h = mpv_create();
    if (!h) { return false; }

    try {
        // M4.0: setOption() で積まれた init 前オプションを適用する。
        for (const auto& opt : std::as_const(pendingOptions_)) {
            checkMpvError(mpv_set_option_string(h,
                opt.name.toUtf8().constData(),
                opt.value.toUtf8().constData()));
        }

        // Render API を使うには vo=libmpv を明示設定する必要がある。
        //   - wid は設定しない（Render API では子 HWND を作らない）
        //   - vo=libmpv にしないと mpv 既定の vo=gpu-next が選ばれ、フレームが
        //     render context に供給されず映像が出ない（黒画面・update callback が
        //     初回1回しか来ない）。render_context_create だけでは vo は上書きされない。
        checkMpvError(mpv_set_option_string(h, "vo", "libmpv"));

        // ファイル未指定でもプロセスを終了しない
        checkMpvError(mpv_set_option_string(h, "idle", "yes"));

        // キー入力は Qt が一手に集約する（QOpenGLWidget のキー横取りを防ぐ）
        checkMpvError(mpv_set_option_string(h, "input-default-bindings", "no"));
        checkMpvError(mpv_set_option_string(h, "input-vo-keyboard",      "no"));

        // キャッシュ（実況用の控えめな有界値）
        checkMpvError(mpv_set_option_string(h, "cache",                  "yes"));
        checkMpvError(mpv_set_option_string(h, "demuxer-max-bytes",      "32MiB"));
        checkMpvError(mpv_set_option_string(h, "demuxer-max-back-bytes", "4MiB"));

        // コンソールへの直接出力を無効化（ログは Qt 側で受け取る）
        checkMpvError(mpv_set_option_string(h, "terminal", "no"));

        // wakeup callback を設定してから initialize する
        mpv_set_wakeup_callback(h, wakeupCallback, this);

        checkMpvError(mpv_initialize(h));

        // INFO レベル以上のログを要求
        checkMpvError(mpv_request_log_messages(h, "info"));

        // watchdog 用プロパティ監視を登録する
        checkMpvError(mpv_observe_property(h, 0, "core-idle",          MPV_FORMAT_FLAG));
        checkMpvError(mpv_observe_property(h, 0, "demuxer-cache-time", MPV_FORMAT_DOUBLE));

        // M4.0: 映像サイズ（アスペクト適用後）の監視
        checkMpvError(mpv_observe_property(h, 0, "dwidth",  MPV_FORMAT_INT64));
        checkMpvError(mpv_observe_property(h, 0, "dheight", MPV_FORMAT_INT64));

        // HTTP ライブストリームの lavf 自動再接続を試みる。
        // 注意: オプション名と値は mpv のバージョン・ビルドによって異なる可能性がある。
        //       エラーは致命的ではないためログ出力のみで続行する。
        {
            const int err = mpv_set_option_string(
                h, "stream-lavf-o",
                "reconnect=1,reconnect_streamed=1,reconnect_delay_max=5");
            if (err < 0) {
                // このバージョンの mpv / ffmpeg では未サポートの可能性があるため無視する
                (void)err;
            }
        }

    } catch (const std::exception&) {
        mpv_terminate_destroy(h);
        return false;
    }

    mpv_ = h;
    return true;
}

void MpvBackend::load(const QString& url) {
    if (!mpv_) { return; }
    QByteArray urlUtf8 = url.toUtf8();
    const char* args[] = {"loadfile", urlUtf8.constData(), nullptr};
    mpv_command_async(mpv_, 0, args);
}

void MpvBackend::command(const QStringList& args) {
    if (!mpv_) { return; }

    std::vector<QByteArray> byteArgs;
    byteArgs.reserve(static_cast<size_t>(args.size()));
    for (const auto& a : args) {
        byteArgs.push_back(a.toUtf8());
    }

    std::vector<const char*> cArgs;
    cArgs.reserve(byteArgs.size() + 1);
    for (const auto& b : byteArgs) {
        cArgs.push_back(b.constData());
    }
    cArgs.push_back(nullptr);

    mpv_command_async(mpv_, 0, cArgs.data());
}

// ---- Render API -------------------------------------------------------

bool MpvBackend::initRenderContext(void* (*getProcAddress)(void*, const char*), void* procAddrCtx) {
    if (!mpv_ || renderCtx_) { return false; }  // mpv 未 init / 二重 init は不可

    mpv_opengl_init_params glInitParams{};
    glInitParams.get_proc_address     = getProcAddress;
    glInitParams.get_proc_address_ctx = procAddrCtx;

    // API タイプを文字列で渡す（MPV_RENDER_API_TYPE_OPENGL = "opengl"）。
    // params 配列は MPV_RENDER_PARAM_INVALID で終端する。
    mpv_render_param params[] = {
        {MPV_RENDER_PARAM_API_TYPE,           const_cast<char*>(MPV_RENDER_API_TYPE_OPENGL)},
        {MPV_RENDER_PARAM_OPENGL_INIT_PARAMS, &glInitParams},
        {MPV_RENDER_PARAM_INVALID,            nullptr}
    };

    if (mpv_render_context_create(&renderCtx_, mpv_, params) < 0) {
        renderCtx_ = nullptr;
        return false;
    }

    // update callback を登録する（内部スレッドから呼ばれる）。
    mpv_render_context_set_update_callback(renderCtx_, renderUpdateCallback, this);
    return true;
}

void MpvBackend::renderFrame(int fboId, int w, int h) {
    if (!renderCtx_) { return; }  // initRenderContext 前は no-op（黒画面のまま）

    mpv_opengl_fbo fbo{};
    fbo.fbo             = fboId;  // Qt 管理 FBO（0 ではない; defaultFramebufferObject()）
    fbo.w               = w;      // 物理ピクセル（devicePixelRatio() 適用済みの値を受け取る）
    fbo.h               = h;
    fbo.internal_format = 0;      // 0 = GL_RGBA8 相当（未知扱い）

    // QOpenGLWidget は Y 座標が反転しているため flip_y=1 が必要。
    int flipY = 1;

    mpv_render_param params[] = {
        {MPV_RENDER_PARAM_OPENGL_FBO, &fbo},
        {MPV_RENDER_PARAM_FLIP_Y,     &flipY},
        {MPV_RENDER_PARAM_INVALID,    nullptr}
    };

    mpv_render_context_render(renderCtx_, params);
}

void MpvBackend::destroyRenderContext() {
    if (!renderCtx_) { return; }

    // update callback を先に無効化してから解放する（新たなコールバックの到着を防ぐ）。
    mpv_render_context_set_update_callback(renderCtx_, nullptr, nullptr);
    mpv_render_context_free(renderCtx_);
    renderCtx_ = nullptr;
}

// -----------------------------------------------------------------------

void MpvBackend::wakeupCallback(void* ctx) {
    // 内部スレッドから呼ばれる。mpv API は一切呼ばず、GUI スレッドへキューイングのみ。
    QMetaObject::invokeMethod(static_cast<MpvBackend*>(ctx),
                              "onWakeup",
                              Qt::QueuedConnection);
}

void MpvBackend::renderUpdateCallback(void* ctx) {
    // 内部スレッドから呼ばれる。mpv_render_* API は一切呼ばず、GUI スレッドへ委譲のみ。
    // （wakeupCallback と対称のパターン）
    auto* self = static_cast<MpvBackend*>(ctx);
    QMetaObject::invokeMethod(self, "onRenderUpdate", Qt::QueuedConnection);
}

void MpvBackend::onRenderUpdate() {
    // GUI スレッドで実行される（render API を呼ぶスレッド = GL コンテキストを持つスレッド）。
    // update callback 自身からではなくキュー経由で呼ぶこと、という mpv の制約を満たす。
    if (!renderCtx_) { return; }

    // mpv_render_context_update() でダーティフラグをクリアして次回コールバックを再アームする。
    // これを呼ばないと最初の1回しかコールバックが来ず映像が更新されない。
    const uint64_t flags = mpv_render_context_update(renderCtx_);
    if (flags & MPV_RENDER_UPDATE_FRAME) {
        emit frameReady();  // → VideoHostWidget::update() → paintGL() → renderFrame()
    }
}

void MpvBackend::onWakeup() {
    // GUI スレッドで mpv_wait_event(0) ループを回してイベントを排出する。
    while (mpv_) {
        mpv_event* event = mpv_wait_event(mpv_, 0);
        if (event->event_id == MPV_EVENT_NONE) { break; }

        switch (event->event_id) {
        case MPV_EVENT_FILE_LOADED:
            emit fileLoaded();
            break;

        case MPV_EVENT_END_FILE: {
            auto* ef = static_cast<mpv_event_end_file*>(event->data);
            emit endFile(static_cast<int>(ef->reason));
            break;
        }

        case MPV_EVENT_LOG_MESSAGE: {
            auto* msg = static_cast<mpv_event_log_message*>(event->data);
            emit logMessage(QString::fromUtf8(msg->prefix),
                            QString::fromUtf8(msg->level),
                            QString::fromUtf8(msg->text).trimmed());
            break;
        }

        case MPV_EVENT_PROPERTY_CHANGE: {
            auto* prop = static_cast<mpv_event_property*>(event->data);
            if (!prop || prop->data == nullptr) { break; }

            if (QLatin1String(prop->name) == QLatin1String("core-idle")) {
                if (prop->format == MPV_FORMAT_FLAG) {
                    const bool idle = (*static_cast<int*>(prop->data) != 0);
                    emit coreIdleChanged(idle);
                }
            } else if (QLatin1String(prop->name) == QLatin1String("demuxer-cache-time")) {
                if (prop->format == MPV_FORMAT_DOUBLE) {
                    const double t = *static_cast<double*>(prop->data);
                    emit cacheTimeChanged(t);
                }
            } else if (QLatin1String(prop->name) == QLatin1String("dwidth")) {
                // M4.0: アスペクト適用後の表示幅（横断決定 3）
                if (prop->format == MPV_FORMAT_INT64) {
                    lastVideoW_ = static_cast<int>(*static_cast<int64_t*>(prop->data));
                    if (lastVideoW_ > 0 && lastVideoH_ > 0) {
                        emit videoSizeChanged(lastVideoW_, lastVideoH_);
                    }
                }
            } else if (QLatin1String(prop->name) == QLatin1String("dheight")) {
                // M4.0: アスペクト適用後の表示高さ（横断決定 3）
                if (prop->format == MPV_FORMAT_INT64) {
                    lastVideoH_ = static_cast<int>(*static_cast<int64_t*>(prop->data));
                    if (lastVideoW_ > 0 && lastVideoH_ > 0) {
                        emit videoSizeChanged(lastVideoW_, lastVideoH_);
                    }
                }
            }
            break;
        }

        case MPV_EVENT_SHUTDOWN: {
            // mpv が内部的に終了（"quit" コマンド等）。ハンドルを解放する。
            mpv_handle* h = std::exchange(mpv_, nullptr);
            mpv_destroy(h);
            return;  // ループ終了（mpv_ が null なので while 条件も偽になる）
        }

        default:
            break;
        }
    }
}

// ---- M4.0: プロパティ/オプション API ----------------------------------------

void MpvBackend::setOption(const QString& name, const QString& value) {
    // init 済み（attach() 完了後）なら no-op。
    if (mpv_) { return; }
    pendingOptions_.append({name, value});
}

void MpvBackend::setProperty(const QString& name, const QString& value) {
    if (!mpv_) { return; }
    const QByteArray n = name.toUtf8();
    const QByteArray v = value.toUtf8();
    mpv_set_property_string(mpv_, n.constData(), v.constData());
}

void MpvBackend::setPropertyFlag(const QString& name, bool on) {
    if (!mpv_) { return; }
    const QByteArray n = name.toUtf8();
    int val = on ? 1 : 0;
    mpv_set_property(mpv_, n.constData(), MPV_FORMAT_FLAG, &val);
}

void MpvBackend::setPropertyDouble(const QString& name, double v) {
    if (!mpv_) { return; }
    const QByteArray n = name.toUtf8();
    mpv_set_property(mpv_, n.constData(), MPV_FORMAT_DOUBLE, &v);
}

double MpvBackend::getPropertyDouble(const QString& name) const {
    if (!mpv_) { return 0.0; }
    const QByteArray n = name.toUtf8();
    double val = 0.0;
    mpv_get_property(mpv_, n.constData(), MPV_FORMAT_DOUBLE, &val);
    return val;
}

QString MpvBackend::getPropertyString(const QString& name) const {
    if (!mpv_) { return {}; }
    const QByteArray n = name.toUtf8();
    char* val = nullptr;
    const int err = mpv_get_property(mpv_, n.constData(), MPV_FORMAT_STRING, &val);
    if (err < 0 || !val) { return {}; }
    QString result = QString::fromUtf8(val);
    mpv_free(val);
    return result;
}

// -----------------------------------------------------------------------------

}  // namespace yapcr::player
