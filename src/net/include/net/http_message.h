#pragma once

#include <QByteArray>
#include <QList>
#include <QNetworkCookie>
#include <QPair>
#include <QString>
#include <QUrl>

namespace yapcr::net {

enum class HttpMethod {
    Get,
    Post,
};

// HTTP リクエスト記述子。HttpClient::send() に渡す。
// headers は既定値（User-Agent / Accept-Encoding / Content-Type）を上書きできる。
// Accept-Encoding="" を渡すと gzip 要求を抑制する（Range 差分取得に必要、M3.3）。
struct HttpRequest {
    HttpMethod method      = HttpMethod::Get;
    QUrl       url;
    QByteArray body;        // POST 本文（urlencoded 済み）
    // POST 時の Content-Type。GET 時は無視される。
    QByteArray contentType = "application/x-www-form-urlencoded";
    // 追加/上書きするリクエストヘッダ。同名既定を上書きする場合は空値を使う。
    QList<QPair<QByteArray, QByteArray>> headers;
};

// HTTP レスポンス記述子。HttpClient::finished(HttpResponse) で渡される。
struct HttpResponse {
    // トランスポート成功 = HTTP 応答を受信できた（statusCode > 0）。
    // false のときは到達不能/タイムアウト等でサーバからの応答なし。
    bool       ok          = false;
    // HTTP ステータスコード。トランスポート失敗時は 0。
    int        statusCode  = 0;
    // statusCode == 304（更新なし）のとき true。
    bool       notModified = false;
    // レスポンス本文。4xx/5xx でも保持する（M3.5 の書き込み確認/エラーページ抽出に必要）。
    QByteArray body;
    // Last-Modified ヘッダ生値（条件付き GET の次回 If-Modified-Since 用）。
    QString    lastModified;
    // Set-Cookie ヘッダをパースした Cookie 一覧。
    QList<QNetworkCookie> setCookies;
};

// ---- 純関数（単体テスト可） ----

// HttpRequest からリクエストヘッダ一覧を組み立てる。
// 既定値: User-Agent / Accept-Encoding: gzip, deflate / POST の Content-Type。
// req.headers にある同名キー（大文字小文字不問）は既定を上書きする。
// req.headers に空値 ("") のキーを渡すと既定ヘッダを送信しない（gzip 無効化等に利用）。
//
// ※ M5 で User-Agent を config 化する。// M5: config 化
QList<QPair<QByteArray, QByteArray>> buildRequestHeaders(const HttpRequest& req);

// QNAM から取り出した素材を HttpResponse に変換する純関数。
// transportOk: HTTP 応答を受信できた（true = statusCode が有効）。
// statusCode: QNetworkReply::attribute(HttpStatusCodeAttribute)。取得できなければ 0。
// respHeaders: QNetworkReply::rawHeaderPairs()。
// body: QNetworkReply::readAll()。
HttpResponse interpretResponse(bool transportOk,
                               int  statusCode,
                               const QList<QPair<QByteArray, QByteArray>>& respHeaders,
                               const QByteArray& body);

}  // namespace yapcr::net
