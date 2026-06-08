#include "peercast/stream_resolver.h"

#include "net/http_client.h"

#include <QRegularExpression>

namespace yapcr::peercast {

StreamResolver::StreamResolver(QObject* parent)
    : QObject(parent)
    , http_(new net::HttpClient(this))
{
    connect(http_, &net::HttpClient::finished,
            this,  &StreamResolver::onHttpFinished);
}

void StreamResolver::resolve(const QString& plsUrl) {
    http_->get(QUrl(plsUrl));
}

void StreamResolver::onHttpFinished(const QByteArray& data, bool ok) {
    if (!ok || data.isEmpty()) {
        emit failed();
        return;
    }

    const QString streamUrl = analyse(data);
    if (streamUrl.isEmpty()) {
        emit failed();
        return;
    }

    emit resolved(streamUrl);
}

QString StreamResolver::analyse(const QByteArray& playlist) {
    // PCRPlayer PeerCast::analyse() の移植。
    // 原典(boost::xpressive):
    //   L"[0-9a-z]+:\\/\\/[^\\/]+\\/stream\\/[0-9a-zA-Z]{32}(\\.[_0-9a-zA-Z]+)?"
    static const QRegularExpression rx(
        QStringLiteral(R"([0-9a-z]+://[^/]+/stream/[0-9a-zA-Z]{32}(\.[_0-9a-zA-Z]+)?)"));

    // プレイリストは UTF-8 として扱う（PeerCastStation は UTF-8 で送信）
    const QString text = QString::fromUtf8(playlist);
    const auto m = rx.match(text);
    if (!m.hasMatch()) {
        return {};
    }
    return m.captured(0);
}

}  // namespace yapcr::peercast
