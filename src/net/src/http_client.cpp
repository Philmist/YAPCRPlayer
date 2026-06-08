#include "net/http_client.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>

namespace yapcr::net {

// PCRPlayer の DEFAULT_USERAGENT に倣ったユーザーエージェント
static constexpr char kUserAgent[] = "YAPCRPlayer/0.1";

HttpClient::HttpClient(QObject* parent)
    : QObject(parent)
    , nam_(new QNetworkAccessManager(this))
{}

void HttpClient::get(const QUrl& url) {
    // 前のリクエストが残っていれば中断する
    if (reply_) {
        reply_->abort();
        reply_->deleteLater();
        reply_ = nullptr;
    }

    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::UserAgentHeader, kUserAgent);
    // gzip/deflate を要求する（QNAM が自動展開する）
    req.setRawHeader("Accept-Encoding", "gzip, deflate");

    reply_ = nam_->get(req);
    connect(reply_, &QNetworkReply::finished,
            this,   &HttpClient::onReplyFinished);
}

void HttpClient::onReplyFinished() {
    if (!reply_) { return; }

    const bool ok = (reply_->error() == QNetworkReply::NoError);
    const QByteArray data = ok ? reply_->readAll() : QByteArray{};

    reply_->deleteLater();
    reply_ = nullptr;

    emit finished(data, ok);
}

}  // namespace yapcr::net
