#pragma once

#include <QObject>
#include <QString>

namespace yapcr::net {
class HttpClient;
}

namespace yapcr::peercast {

// PeerCast /pls/ URL から /stream/ URL を非同期解決する。
// PCRPlayer PeerCast::connect() + PeerCast::analyse() の Qt 移植。
//
// 使い方:
//   auto* r = new StreamResolver(parent);
//   connect(r, &StreamResolver::resolved, ...);
//   connect(r, &StreamResolver::failed,   ...);
//   r->resolve("http://host:port/pls/<32hex>?...");
class StreamResolver : public QObject {
    Q_OBJECT

public:
    explicit StreamResolver(QObject* parent = nullptr);

    // pls_url を GET して stream URL を非同期解決する。
    void resolve(const QString& plsUrl);

signals:
    // 解決成功。streamUrl は mpv に直接渡せる形式。
    void resolved(const QString& streamUrl);

    // 解決失敗（ネットワークエラーまたは stream URL が見つからない）。
    void failed();

private slots:
    void onHttpFinished(const QByteArray& data, bool ok);

private:
    // プレイリスト本文から stream URL を抽出する。
    // PCRPlayer PeerCast::analyse() の移植。
    // 原典: L"[0-9a-z]+:\\/\\/[^\\/]+\\/stream\\/[0-9a-zA-Z]{32}(\\.[_0-9a-zA-Z]+)?"
    static QString analyse(const QByteArray& playlist);

    net::HttpClient* http_{nullptr};
};

}  // namespace yapcr::peercast
