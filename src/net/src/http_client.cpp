#include "net/http_client.h"

#include <QNetworkAccessManager>
#include <QNetworkCookieJar>
#include <QNetworkReply>
#include <QNetworkRequest>

namespace yapcr::net {

HttpClient::HttpClient(QObject* parent)
    : QObject(parent)
    , nam_(new QNetworkAccessManager(this))
{
    // Cookie jar を明示設定する。QNAM 既定では jar が無効な実装があるため。
    // 同一 HttpClient を使いまわすことで M3.5 の確認 Cookie を半自動保持する。
    nam_->setCookieJar(new QNetworkCookieJar(nam_));
}

void HttpClient::get(const QUrl& url) {
    HttpRequest req;
    req.method = HttpMethod::Get;
    req.url    = url;
    send(req);
}

void HttpClient::post(const QUrl& url,
                      const QByteArray& body,
                      const QList<QPair<QByteArray, QByteArray>>& extraHeaders) {
    HttpRequest req;
    req.method  = HttpMethod::Post;
    req.url     = url;
    req.body    = body;
    req.headers = extraHeaders;
    send(req);
}

void HttpClient::send(const HttpRequest& req) {
    // 前のリクエストが残っていれば中断する
    if (reply_) {
        reply_->abort();
        reply_->deleteLater();
        reply_ = nullptr;
    }

    QNetworkRequest netReq(req.url);
    const auto headers = buildRequestHeaders(req);
    for (const auto& [key, value] : headers) {
        netReq.setRawHeader(key, value);
    }

    if (req.method == HttpMethod::Post) {
        reply_ = nam_->post(netReq, req.body);
    } else {
        reply_ = nam_->get(netReq);
    }

    connect(reply_, &QNetworkReply::finished,
            this,   &HttpClient::onReplyFinished);
}

void HttpClient::onReplyFinished() {
    if (!reply_) { return; }

    // statusCode が取れれば HTTP 応答ありとみなす（4xx/5xx でも ok=true）。
    // 取れない場合（到達不能/タイムアウト等）は transportOk=false。
    const QVariant statusVar =
        reply_->attribute(QNetworkRequest::HttpStatusCodeAttribute);
    const bool transportOk  = statusVar.isValid();
    const int  statusCode   = transportOk ? statusVar.toInt() : 0;

    // 本文は ok に関わらず常に読む（M3.5 の確認/エラーページ抽出に必要）
    const QByteArray body = reply_->readAll();

    const HttpResponse resp =
        interpretResponse(transportOk, statusCode,
                          reply_->rawHeaderPairs(), body);

    reply_->deleteLater();
    reply_ = nullptr;

    emit finished(resp);
}

}  // namespace yapcr::net
