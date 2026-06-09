#pragma once

#include "net/http_message.h"

#include <QByteArray>
#include <QObject>
#include <QUrl>

class QNetworkAccessManager;
class QNetworkReply;

namespace yapcr::net {

// QNetworkAccessManager の薄いラッパ。
// M1: GET のみ対応。
// M3.1: POST / カスタムヘッダ / Cookie jar / 条件付き GET（304）に拡張。
// TLS/gzip は QNAM 内蔵で自動処理。
// 同時には 1 リクエストのみを発行する設計。前のリクエストは中断される。
class HttpClient : public QObject {
    Q_OBJECT

public:
    explicit HttpClient(QObject* parent = nullptr);

    // 汎用リクエスト発行。GET/POST ともにこれが基本。
    // 完了時に finished(HttpResponse) を emit する。
    void send(const HttpRequest& req);

    // 便宜: GET リクエストを既定設定で発行する（既存コードとの互換用）。
    void get(const QUrl& url);

    // 便宜: POST リクエストを発行する。
    // extraHeaders: Referer/Cookie 等の追加ヘッダ。
    void post(const QUrl& url,
              const QByteArray& body,
              const QList<QPair<QByteArray, QByteArray>>& extraHeaders = {});

signals:
    // リクエストが完了（またはトランスポートエラー）した。
    // HttpResponse::ok == false の場合はネットワーク到達不能等。
    // HTTP 4xx/5xx は ok==true、statusCode でステータスを確認すること。
    void finished(const yapcr::net::HttpResponse& resp);

private slots:
    void onReplyFinished();

private:
    QNetworkAccessManager* nam_;
    QNetworkReply*         reply_{nullptr};
};

}  // namespace yapcr::net
