#pragma once

#include "net/http_message.h"
#include "peercast/channel_info.h"

#include <QElapsedTimer>
#include <QObject>
#include <QString>

namespace yapcr::net {
class HttpClient;
}

namespace yapcr::peercast {

// PeerCast admin HTTP API の非同期ラッパ。
// PCRPlayer PeerCast クラスの Qt 移植版（同期→非同期）。
//
// admin API エンドポイント:
//   http://host:port/admin?cmd=bump&id=<ID>   — 上流ソース切替要求
//   http://host:port/admin?cmd=stop&id=<ID>   — 配信停止要求
//   http://host:port/admin?cmd=viewxml         — チャンネル情報取得（XML）
//
// HttpClient は用途ごとに独立インスタンスを持つ（単一リクエストのみの制約を回避）:
//   infoClient_  — 定期 viewxml ポーリング
//   guardClient_ — relay ガード確認用 viewxml
//   cmdClient_   — bump/stop の fire-and-forget
class PeerCastController : public QObject {
    Q_OBJECT

public:
    // host, port, id は PeerCastUrl::parse() の結果から渡す。
    explicit PeerCastController(const QString& host,
                                 const QString& port,
                                 const QString& id,
                                 QObject* parent = nullptr);
    ~PeerCastController() override;

    // viewxml を非同期取得する。完了時に channelInfo() / infoFailed() を emit する。
    void requestInfo();

    // bump（上流ソース切替要求）を発行する。
    // guardByRelay=true なら先に viewxml を取得して relays==0 を確認してから bump。
    // PCRPlayer の bump() レート制限（60 秒）を適用する。
    void bump(bool guardByRelay = false);

    // stop（配信停止要求）を発行する。
    // guardByRelay=true なら先に viewxml を取得して relays==0 を確認してから stop。
    void stop(bool guardByRelay = false);

signals:
    // viewxml の取得とパースが成功した。
    void channelInfo(const yapcr::peercast::ChannelInfo& info);

    // viewxml の取得またはパースに失敗した。
    void infoFailed();

    // bump コマンドが発行された（レート制限・ガード通過後）。
    void bumped();

    // stop コマンドが発行された。
    void stopped();

    // エラーまたは操作抑止の通知（レート制限・ガード拒否など）。
    void controlError(const QString& reason);

private slots:
    void onInfoFinished(const yapcr::net::HttpResponse& resp);
    void onGuardBumpFinished(const yapcr::net::HttpResponse& resp);
    void onGuardStopFinished(const yapcr::net::HttpResponse& resp);

private:
    QUrl adminUrl(const QString& cmd) const;  // ?cmd=<cmd>&id=<id>
    QUrl viewxmlUrl() const;                   // ?cmd=viewxml
    void issueBump();
    void issueStop();

    QString host_;
    QString port_;
    QString id_;

    net::HttpClient* infoClient_;   // viewxml 定期ポーリング用
    net::HttpClient* guardClient_;  // relay ガード用
    net::HttpClient* cmdClient_;    // bump/stop fire-and-forget 用

    QElapsedTimer lastBump_;  // PCRPlayer の timer(true) 相当（60 秒レート制限）

    static constexpr qint64 kBumpRateLimitMs = 60'000;
};

}  // namespace yapcr::peercast
