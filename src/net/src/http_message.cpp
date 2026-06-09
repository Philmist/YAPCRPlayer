#include "net/http_message.h"

#include <QNetworkCookie>

namespace yapcr::net {

// PCRPlayer DEFAULT_USERAGENT に倣う。// M5: config 化
static constexpr char kUserAgent[] = "YAPCRPlayer/0.1";

QList<QPair<QByteArray, QByteArray>> buildRequestHeaders(const HttpRequest& req) {
    // 既定ヘッダを組み立てる（キーは小文字で管理）
    struct DefaultEntry {
        QByteArray key;
        QByteArray value;
    };
    QList<DefaultEntry> defaults;
    defaults.push_back(DefaultEntry{"user-agent", QByteArray(kUserAgent)});
    // accept-encoding は設定しない。QNAM が自動で付加し、gzip 展開も自動処理する。
    // raw header で明示すると QNAM の自動展開が無効化されてしまうため。
    // Range 差分取得（M3.x）では QNetworkRequest::AutoDecompressResponseBodyAttribute を使う。
    if (req.method == HttpMethod::Post) {
        defaults.push_back(DefaultEntry{"content-type", req.contentType});
    }

    // req.headers を小文字化して上書きマップを作る
    QList<QPair<QByteArray, QByteArray>> overrides;
    for (const auto& pair : req.headers) {
        overrides.push_back({pair.first.toLower(), pair.second});
    }

    QList<QPair<QByteArray, QByteArray>> result;

    // 既定ヘッダを出力（上書き/無効化を適用）
    for (const auto& def : defaults) {
        bool overridden = false;
        for (const auto& ov : overrides) {
            if (ov.first == def.key) {
                overridden = true;
                if (!ov.second.isEmpty()) {
                    // 上書き値で追加（キーは元の小文字のまま）
                    result.push_back({def.key, ov.second});
                }
                // 空値 = このヘッダを送らない
                break;
            }
        }
        if (!overridden) {
            result.push_back({def.key, def.value});
        }
    }

    // req.headers のうち既定にないカスタムヘッダを追加（元のキー名・大文字保持）
    for (const auto& origPair : req.headers) {
        const QByteArray lk = origPair.first.toLower();
        bool isDefault = false;
        for (const auto& def : defaults) {
            if (def.key == lk) { isDefault = true; break; }
        }
        if (!isDefault && !origPair.second.isEmpty()) {
            result.push_back({origPair.first, origPair.second});
        }
    }

    return result;
}

HttpResponse interpretResponse(bool transportOk,
                               int  statusCode,
                               const QList<QPair<QByteArray, QByteArray>>& respHeaders,
                               const QByteArray& body) {
    HttpResponse resp;
    resp.ok          = transportOk;
    resp.statusCode  = statusCode;
    resp.notModified = (statusCode == 304);
    resp.body        = body;

    for (const auto& pair : respHeaders) {
        const QByteArray lk = pair.first.toLower();
        if (lk == "last-modified") {
            resp.lastModified = QString::fromLatin1(pair.second);
        } else if (lk == "set-cookie") {
            const auto cookies = QNetworkCookie::parseCookies(pair.second);
            resp.setCookies.append(cookies);
        }
    }

    return resp;
}

}  // namespace yapcr::net
