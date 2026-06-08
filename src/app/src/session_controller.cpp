#include "session_controller.h"

#include "player/mpv_backend.h"
#include "peercast/peercast_url.h"
#include "peercast/stream_resolver.h"

#include <QDebug>

namespace yapcr::app {

SessionController::SessionController(player::MpvBackend* backend, QObject* parent)
    : QObject(parent)
    , backend_(backend)
{}

SessionController::~SessionController() = default;

void SessionController::start(const QString& path,
                               const QString& name,
                               const QString& contact,
                               bool           commandline)
{
    path_        = path;
    name_        = name;
    contact_     = contact;
    commandline_ = commandline;

    // タイトルを更新する（name が空の場合は path をそのまま表示）
    emit titleChanged(name.isEmpty() ? path : name);

    // /pls/ URL か判定する
    const auto peca = peercast::PeerCastUrl::parse(path);
    if (peca.valid) {
        // 非同期で /stream/ URL を解決してから mpv に load する
        emit statusMessage(tr("PeerCast URL を解決中..."));

        // 前の resolver があれば破棄してから新たに作成する
        if (resolver_) {
            resolver_->deleteLater();
        }
        resolver_ = new peercast::StreamResolver(this);
        connect(resolver_, &peercast::StreamResolver::resolved,
                this,       &SessionController::onStreamResolved);
        connect(resolver_, &peercast::StreamResolver::failed,
                this,       &SessionController::onStreamFailed);
        resolver_->resolve(path);
    } else {
        // ローカルファイルまたは直 URL — 直接 mpv に渡す
        backend_->load(path);
        emit statusMessage(tr("再生中: %1").arg(path));
    }
}

void SessionController::onStreamResolved(const QString& streamUrl) {
    qDebug() << "[peercast] stream URL:" << streamUrl;
    emit statusMessage(tr("接続中: %1").arg(streamUrl));
    backend_->load(streamUrl);
}

void SessionController::onStreamFailed() {
    qDebug() << "[peercast] stream URL 解決失敗";
    emit statusMessage(tr("PeerCast URL の解決に失敗しました"));
}

}  // namespace yapcr::app
