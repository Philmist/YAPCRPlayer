#pragma once

#include <QByteArray>
#include <QObject>
#include <QUrl>

class QNetworkAccessManager;
class QNetworkReply;

namespace yapcr::net {

// QNetworkAccessManager の薄いラッパ。
// M1: GET のみ対応。TLS/Cookie/gzip は QNAM 内蔵で自動処理。
// 同時には 1 リクエストのみを発行する設計。
class HttpClient : public QObject {
    Q_OBJECT

public:
    explicit HttpClient(QObject* parent = nullptr);

    // url を非同期 GET する。完了時に finished() を emit する。
    // 前のリクエストが完了していない場合は中断してから新しいリクエストを発行する。
    void get(const QUrl& url);

signals:
    // リクエストが完了した。ok=false の場合はネットワークエラー。
    void finished(const QByteArray& data, bool ok);

private slots:
    void onReplyFinished();

private:
    QNetworkAccessManager* nam_;
    QNetworkReply*         reply_{nullptr};
};

}  // namespace yapcr::net
