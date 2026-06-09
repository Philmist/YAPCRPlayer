#include "peercast/peercast_controller.h"
#include "peercast/channel_info.h"
#include "net/http_client.h"

#include <QDebug>
#include <QUrl>
#include <QUrlQuery>

namespace yapcr::peercast {

PeerCastController::PeerCastController(const QString& host,
                                        const QString& port,
                                        const QString& id,
                                        QObject* parent)
    : QObject(parent)
    , host_(host)
    , port_(port)
    , id_(id)
    , infoClient_(new net::HttpClient(this))
    , guardClient_(new net::HttpClient(this))
    , cmdClient_(new net::HttpClient(this))
{
    connect(infoClient_, &net::HttpClient::finished,
            this,        &PeerCastController::onInfoFinished);
}

PeerCastController::~PeerCastController() = default;

// ---- URL ヘルパ -----------------------------------------------------------

QUrl PeerCastController::adminUrl(const QString& cmd) const {
    QUrl url;
    url.setScheme(QStringLiteral("http"));
    url.setHost(host_);
    url.setPort(port_.toInt());
    url.setPath(QStringLiteral("/admin"));
    QUrlQuery q;
    q.addQueryItem(QStringLiteral("cmd"), cmd);
    q.addQueryItem(QStringLiteral("id"),  id_);
    url.setQuery(q);
    return url;
}

QUrl PeerCastController::viewxmlUrl() const {
    QUrl url;
    url.setScheme(QStringLiteral("http"));
    url.setHost(host_);
    url.setPort(port_.toInt());
    url.setPath(QStringLiteral("/admin"));
    QUrlQuery q;
    q.addQueryItem(QStringLiteral("cmd"), QStringLiteral("viewxml"));
    url.setQuery(q);
    return url;
}

// ---- 公開 API -------------------------------------------------------------

void PeerCastController::requestInfo() {
    infoClient_->get(viewxmlUrl());
}

void PeerCastController::bump(bool guardByRelay) {
    // PCRPlayer の timer(true) レート制限相当（60 秒間隔）
    if (lastBump_.isValid() && lastBump_.elapsed() < kBumpRateLimitMs) {
        const qint64 remaining = kBumpRateLimitMs - lastBump_.elapsed();
        emit controlError(
            tr("bump: レート制限 (あと %1 秒)").arg(remaining / 1000));
        return;
    }

    if (!guardByRelay) {
        issueBump();
        return;
    }

    // relay ガード: viewxml を取得してリレー数を確認してから bump
    connect(guardClient_, &net::HttpClient::finished,
            this,         &PeerCastController::onGuardBumpFinished,
            Qt::SingleShotConnection);
    guardClient_->get(viewxmlUrl());
}

void PeerCastController::stop(bool guardByRelay) {
    if (!guardByRelay) {
        issueStop();
        return;
    }

    connect(guardClient_, &net::HttpClient::finished,
            this,         &PeerCastController::onGuardStopFinished,
            Qt::SingleShotConnection);
    guardClient_->get(viewxmlUrl());
}

// ---- 内部発行 -------------------------------------------------------------

void PeerCastController::issueBump() {
    cmdClient_->get(adminUrl(QStringLiteral("bump")));
    lastBump_.start();  // PCRPlayer の timer(true) に相当
    emit bumped();
    qDebug() << "[peercast] bump issued:"
             << QStringLiteral("http://%1:%2/admin?cmd=bump&id=%3")
                    .arg(host_, port_, id_);
}

void PeerCastController::issueStop() {
    cmdClient_->get(adminUrl(QStringLiteral("stop")));
    emit stopped();
    qDebug() << "[peercast] stop issued";
}

// ---- slots ---------------------------------------------------------------

void PeerCastController::onInfoFinished(const net::HttpResponse& resp) {
    if (!resp.ok || resp.body.isEmpty()) {
        emit infoFailed();
        return;
    }
    const ChannelInfo info = parseViewXml(resp.body, id_);
    if (info.valid) {
        emit channelInfo(info);
    } else {
        // viewxml は取れたがチャンネルが見つからなかった
        emit infoFailed();
    }
}

void PeerCastController::onGuardBumpFinished(const net::HttpResponse& resp) {
    if (!resp.ok || resp.body.isEmpty()) {
        // viewxml 取得失敗 → ガードを通して bump する（PCRPlayer の挙動に準拠）
        issueBump();
        return;
    }
    const ChannelInfo info = parseViewXml(resp.body, id_);
    // relays > 0: 他にリレーしているノードがいる → bump を抑止
    // PCRPlayer check(relay) = info() && relays==0 に相当
    if (info.valid && info.relays > 0) {
        emit controlError(
            tr("bump 抑止: 他リレーあり (relays=%1)").arg(info.relays));
        return;
    }
    issueBump();
}

void PeerCastController::onGuardStopFinished(const net::HttpResponse& resp) {
    if (!resp.ok || resp.body.isEmpty()) {
        issueStop();
        return;
    }
    const ChannelInfo info = parseViewXml(resp.body, id_);
    if (info.valid && info.relays > 0) {
        emit controlError(
            tr("stop 抑止: 他リレーあり (relays=%1)").arg(info.relays));
        return;
    }
    issueStop();
}

}  // namespace yapcr::peercast
