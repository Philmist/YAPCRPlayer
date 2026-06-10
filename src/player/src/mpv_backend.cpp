#include "player/mpv_backend.h"

#include <mpv/client.h>

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
        // onWakeup が後でキュー消化されても mpv_ は null なので安全。
        mpv_set_wakeup_callback(mpv_, nullptr, nullptr);
        mpv_handle* h = std::exchange(mpv_, nullptr);
        mpv_terminate_destroy(h);
    }
}

bool MpvBackend::attach(quintptr wid) {
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

        // 映像を Qt ウィジェットの HWND に埋め込む（show() 後に呼ぶこと）。
        // ref: mpv/client.h — wid は MPV_FORMAT_INT64 で渡す。
        auto widInt = static_cast<int64_t>(wid);
        checkMpvError(mpv_set_option(h, "wid", MPV_FORMAT_INT64, &widInt));

        // 映像レンダラ（Windows では gpu-next + d3d11 が既定になる）
        checkMpvError(mpv_set_option_string(h, "vo", "gpu-next"));

        // ファイル未指定でもプロセスを終了しない
        checkMpvError(mpv_set_option_string(h, "idle", "yes"));

        // キー入力は Qt が一手に集約する（--wid 子窓のキー横取りを防ぐ）
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
        // PCRPlayer の architecture-decisions.md Q7 「mpv cache + lavf reconnect」に対応。
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

void MpvBackend::wakeupCallback(void* ctx) {
    // 内部スレッドから呼ばれる。mpv API は一切呼ばず、GUI スレッドへキューイングのみ。
    QMetaObject::invokeMethod(static_cast<MpvBackend*>(ctx),
                              "onWakeup",
                              Qt::QueuedConnection);
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
