#include "session_controller.h"

#include "player/mpv_backend.h"
#include "player/watchdog.h"
#include "peercast/peercast_url.h"
#include "peercast/peercast_controller.h"
#include "peercast/channel_info.h"
#include "peercast/stream_resolver.h"

#include <QDateTime>
#include <QDebug>
#include <QTimer>

namespace yapcr::app {

SessionController::SessionController(player::MpvBackend* backend, QObject* parent)
    : QObject(parent)
    , backend_(backend)
{}

SessionController::~SessionController() = default;

// ---- 公開 API -------------------------------------------------------------

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

    // 前の PeerCast セッションリソースを破棄する
    teardownPeerCast();

    // /pls/ URL か判定する
    const auto peca = peercast::PeerCastUrl::parse(path);
    if (peca.valid) {
        plsUrl_ = path;
        // 非同期で /stream/ URL を解決してから mpv に load する
        emit statusMessage(tr("PeerCast URL を解決中..."));

        if (resolver_) { resolver_->deleteLater(); }
        resolver_ = new peercast::StreamResolver(this);
        connect(resolver_, &peercast::StreamResolver::resolved,
                this,       &SessionController::onStreamResolved);
        connect(resolver_, &peercast::StreamResolver::failed,
                this,       &SessionController::onStreamFailed);
        resolver_->resolve(path);
    } else {
        // ローカルファイルまたは直 URL — controller/watchdog は起動しない（CLI 互換）
        plsUrl_.clear();
        streamUrl_.clear();
        backend_->load(path);
        emit statusMessage(tr("再生中: %1").arg(path));
    }
}

// ---- 手動操作スロット --------------------------------------------------------

void SessionController::manualBump() {
    if (!controller_) {
        emit statusMessage(tr("PeerCast セッションがありません"));
        return;
    }
    controller_->bump(/*guardByRelay=*/false);
}

void SessionController::manualStop() {
    if (!controller_) {
        emit statusMessage(tr("PeerCast セッションがありません"));
        return;
    }
    controller_->stop(/*guardByRelay=*/false);
}

void SessionController::manualReload() {
    if (streamUrl_.isEmpty()) {
        emit statusMessage(tr("再読込できる URL がありません"));
        return;
    }
    backend_->load(streamUrl_);
    emit statusMessage(tr("再読込: %1").arg(streamUrl_));
}

// ---- ストリーム解決スロット ---------------------------------------------------

void SessionController::onStreamResolved(const QString& streamUrl) {
    qDebug() << "[peercast] stream URL:" << streamUrl;
    streamUrl_ = streamUrl;
    emit statusMessage(tr("接続中: %1").arg(streamUrl));
    backend_->load(streamUrl);

    // PeerCast セッションを構成する（stream URL 取得後に初回情報取得）
    setupPeerCast();
}

void SessionController::onStreamFailed() {
    qDebug() << "[peercast] stream URL 解決失敗";
    emit statusMessage(tr("PeerCast URL の解決に失敗しました"));
}

// ---- PeerCast セットアップ ---------------------------------------------------

void SessionController::setupPeerCast() {
    // 前のリソースが残っていれば破棄（二重呼び出し保護）
    teardownPeerCast();

    const auto peca = peercast::PeerCastUrl::parse(plsUrl_);
    if (!peca.valid) { return; }

    // PeerCastController
    controller_ = new peercast::PeerCastController(peca.host, peca.port, peca.id, this);
    connect(controller_, &peercast::PeerCastController::channelInfo,
            this,        &SessionController::onChannelInfo);
    connect(controller_, &peercast::PeerCastController::infoFailed,
            this,        &SessionController::onInfoFailed);
    connect(controller_, &peercast::PeerCastController::bumped,
            this,        &SessionController::onBumped);
    connect(controller_, &peercast::PeerCastController::stopped,
            this,        &SessionController::onStopped);
    connect(controller_, &peercast::PeerCastController::controlError,
            this,        &SessionController::onControlError);

    // viewxml 定期ポーリングタイマ（30 秒）
    infoTimer_ = new QTimer(this);
    infoTimer_->setInterval(kInfoPollIntervalMs);
    connect(infoTimer_, &QTimer::timeout, controller_,
            &peercast::PeerCastController::requestInfo);
    infoTimer_->start();

    // Watchdog
    watchdog_ = new player::Watchdog(this);
    connect(backend_, &player::MpvBackend::coreIdleChanged,
            this, [this](bool idle) {
                watchdog_->onCoreIdle(idle, QDateTime::currentMSecsSinceEpoch());
            });
    connect(backend_, &player::MpvBackend::cacheTimeChanged,
            this, [this](double t) {
                watchdog_->onCacheTime(t, QDateTime::currentMSecsSinceEpoch());
            });
    connect(backend_, &player::MpvBackend::fileLoaded,
            this, [this] {
                watchdog_->onFileLoaded(QDateTime::currentMSecsSinceEpoch());
            });
    connect(backend_, &player::MpvBackend::endFile,
            this, [this](int reason) {
                watchdog_->onEndFile(reason, QDateTime::currentMSecsSinceEpoch());
            });

    connect(watchdog_, &player::Watchdog::reloadRequested,
            this,      &SessionController::onReloadRequested);
    connect(watchdog_, &player::Watchdog::bumpRequested,
            this,      &SessionController::onBumpRequested);
    connect(watchdog_, &player::Watchdog::supplyLost,
            this,      &SessionController::onSupplyLost);

    // Watchdog 評価タイマ（3 秒）
    watchdogTimer_ = new QTimer(this);
    watchdogTimer_->setInterval(kWatchdogTickMs);
    connect(watchdogTimer_, &QTimer::timeout, this, [this] {
        watchdog_->evaluate(QDateTime::currentMSecsSinceEpoch());
    });
    watchdogTimer_->start();

    // 初回チャンネル情報取得
    controller_->requestInfo();
}

void SessionController::teardownPeerCast() {
    if (infoTimer_) {
        infoTimer_->stop();
        infoTimer_->deleteLater();
        infoTimer_ = nullptr;
    }
    if (watchdogTimer_) {
        watchdogTimer_->stop();
        watchdogTimer_->deleteLater();
        watchdogTimer_ = nullptr;
    }
    if (watchdog_) {
        watchdog_->deleteLater();
        watchdog_ = nullptr;
    }
    if (controller_) {
        controller_->deleteLater();
        controller_ = nullptr;
    }
    // setupPeerCast() で backend_ から this へのラムダ接続を全て切断する。
    // backend_ は SessionController より長生きするため、dangling ラムダを防ぐ。
    // (QObject::disconnect の static 形式: sender から receiver への全接続を解除)
    QObject::disconnect(backend_, nullptr, this, nullptr);
}

// ---- PeerCastController スロット -------------------------------------------

void SessionController::onChannelInfo(const peercast::ChannelInfo& info) {
    const QString title  = peercast::formatTitle(info);
    const QString status = peercast::formatStatus(info);
    if (!title.isEmpty())  { emit titleChanged(title); }
    if (!status.isEmpty()) { emit statusMessage(status); }
}

void SessionController::onInfoFailed() {
    qDebug() << "[peercast] viewxml 取得失敗";
}

void SessionController::onBumped() {
    emit statusMessage(tr("再接続を要求しました (bump)"));
    // bump 後に stream URL を再 load して新しい上流から受信する
    if (!streamUrl_.isEmpty()) {
        backend_->load(streamUrl_);
    }
}

void SessionController::onStopped() {
    emit statusMessage(tr("配信を停止しました"));
}

void SessionController::onControlError(const QString& reason) {
    qDebug() << "[peercast] controlError:" << reason;
    emit statusMessage(tr("PeerCast エラー: %1").arg(reason));
}

// ---- Watchdog スロット -------------------------------------------------------

void SessionController::onReloadRequested() {
    if (streamUrl_.isEmpty()) { return; }
    qDebug() << "[watchdog→session] reload:" << streamUrl_;
    emit statusMessage(tr("ストリームを再読込しています..."));
    backend_->load(streamUrl_);
}

void SessionController::onBumpRequested() {
    if (!controller_) { return; }
    qDebug() << "[watchdog→session] bump 要求";
    emit statusMessage(tr("上流への再接続を要求しています (auto bump)..."));
    controller_->bump(/*guardByRelay=*/false);
}

void SessionController::onSupplyLost() {
    emit statusMessage(tr("供給が回復しません。手動で操作してください。"));
}

}  // namespace yapcr::app
