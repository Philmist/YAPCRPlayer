#include <QTest>

#include "net/http_message.h"

using namespace yapcr::net;

// helper: ヘッダ一覧からキー（小文字比較）で値を取り出す
static QByteArray headerValue(const QList<QPair<QByteArray, QByteArray>>& headers,
                              const QByteArray& keyLower) {
    for (const auto& [k, v] : headers) {
        if (k.toLower() == keyLower) { return v; }
    }
    return {};
}

class TstNet : public QObject {
    Q_OBJECT

private slots:

    // ======== buildRequestHeaders ========

    void buildHeaders_get_hasUserAgent() {
        HttpRequest req;
        req.method = HttpMethod::Get;
        req.url    = QUrl("http://example.com/");
        const auto h = buildRequestHeaders(req);
        QVERIFY(!headerValue(h, "user-agent").isEmpty());
    }

    void buildHeaders_get_hasGzip() {
        HttpRequest req;
        req.method = HttpMethod::Get;
        req.url    = QUrl("http://example.com/");
        const auto h = buildRequestHeaders(req);
        const QByteArray ae = headerValue(h, "accept-encoding");
        QVERIFY(ae.contains("gzip"));
    }

    void buildHeaders_get_noContentType() {
        // GET には Content-Type を付けない
        HttpRequest req;
        req.method = HttpMethod::Get;
        req.url    = QUrl("http://example.com/");
        const auto h = buildRequestHeaders(req);
        QVERIFY(headerValue(h, "content-type").isEmpty());
    }

    void buildHeaders_post_hasContentType() {
        // POST には Content-Type: application/x-www-form-urlencoded が入る
        HttpRequest req;
        req.method = HttpMethod::Post;
        req.url    = QUrl("http://example.com/write");
        req.body   = "foo=bar";
        const auto h = buildRequestHeaders(req);
        const QByteArray ct = headerValue(h, "content-type");
        QVERIFY(ct.contains("application/x-www-form-urlencoded"));
    }

    void buildHeaders_overrideGzip_empty() {
        // Range 差分取得のため Accept-Encoding を空文字で無効化する（M3.3）
        HttpRequest req;
        req.method  = HttpMethod::Get;
        req.url     = QUrl("http://example.com/dat");
        req.headers = {{"Accept-Encoding", ""}};
        const auto h = buildRequestHeaders(req);
        // Accept-Encoding ヘッダ自体が存在しないこと
        QVERIFY(headerValue(h, "accept-encoding").isEmpty());
        for (const auto& [k, v] : h) {
            QVERIFY(k.toLower() != "accept-encoding");
        }
    }

    void buildHeaders_customHeaders_referer() {
        // Referer/Cookie/If-Modified-Since が追加される
        HttpRequest req;
        req.method  = HttpMethod::Post;
        req.url     = QUrl("http://example.com/write");
        req.headers = {
            {"Referer",          "http://example.com/thread/"},
            {"Cookie",           "NAME=test"},
            {"If-Modified-Since","Mon, 01 Jun 2026 00:00:00 GMT"},
        };
        const auto h = buildRequestHeaders(req);
        QCOMPARE(headerValue(h, "referer"),           QByteArray("http://example.com/thread/"));
        QCOMPARE(headerValue(h, "cookie"),            QByteArray("NAME=test"));
        QCOMPARE(headerValue(h, "if-modified-since"), QByteArray("Mon, 01 Jun 2026 00:00:00 GMT"));
    }

    // ======== interpretResponse ========

    void interpret_200_ok() {
        const QByteArray body = "hello";
        const auto resp = interpretResponse(true, 200, {}, body);
        QVERIFY(resp.ok);
        QCOMPARE(resp.statusCode,  200);
        QVERIFY(!resp.notModified);
        QCOMPARE(resp.body, body);
    }

    void interpret_304_notModified() {
        const auto resp = interpretResponse(true, 304, {}, {});
        QVERIFY(resp.ok);
        QCOMPARE(resp.statusCode, 304);
        QVERIFY(resp.notModified);
    }

    void interpret_404_bodyPreserved() {
        // 4xx でも body を保持する（M3.5 の確認/エラーページ抽出に必要）
        const QByteArray body = "<html>Not Found</html>";
        const auto resp = interpretResponse(true, 404, {}, body);
        QVERIFY(resp.ok);              // HTTP 応答は受信できた
        QCOMPARE(resp.statusCode, 404);
        QCOMPARE(resp.body, body);
    }

    void interpret_500_bodyPreserved() {
        const QByteArray body = "Internal Server Error";
        const auto resp = interpretResponse(true, 500, {}, body);
        QVERIFY(resp.ok);
        QCOMPARE(resp.statusCode, 500);
        QCOMPARE(resp.body, body);
    }

    void interpret_transportError() {
        // 到達不能/タイムアウト等（HTTP 応答なし）
        const auto resp = interpretResponse(false, 0, {}, {});
        QVERIFY(!resp.ok);
        QCOMPARE(resp.statusCode, 0);
    }

    void interpret_lastModified() {
        const QList<QPair<QByteArray, QByteArray>> headers = {
            {"Last-Modified", "Tue, 09 Jun 2026 12:00:00 GMT"},
        };
        const auto resp = interpretResponse(true, 200, headers, {});
        QCOMPARE(resp.lastModified, QString("Tue, 09 Jun 2026 12:00:00 GMT"));
    }

    void interpret_setCookie_single() {
        const QList<QPair<QByteArray, QByteArray>> headers = {
            {"Set-Cookie", "SID=abc123; Path=/"},
        };
        const auto resp = interpretResponse(true, 200, headers, {});
        QCOMPARE(resp.setCookies.size(), 1);
        QCOMPARE(resp.setCookies.at(0).name(), QString("SID"));
        QCOMPARE(resp.setCookies.at(0).value(), QString("abc123"));
    }

    void interpret_setCookie_multiple() {
        // 複数 Set-Cookie ヘッダ（2段階 Cookie フローの確認用）
        const QList<QPair<QByteArray, QByteArray>> headers = {
            {"Set-Cookie", "A=1; Path=/"},
            {"Set-Cookie", "B=2; Path=/"},
        };
        const auto resp = interpretResponse(true, 200, headers, {});
        QCOMPARE(resp.setCookies.size(), 2);
        // 名前のセットが一致するか（順序は問わない）
        QStringList names;
        for (const auto& c : resp.setCookies) { names.append(c.name()); }
        QVERIFY(names.contains("A"));
        QVERIFY(names.contains("B"));
    }

    void interpret_headerKeysAreCaseInsensitive() {
        // ヘッダキーが大文字でも小文字でも動作する
        const QList<QPair<QByteArray, QByteArray>> headers = {
            {"LAST-MODIFIED", "Mon, 01 Jun 2026 00:00:00 GMT"},
            {"SET-COOKIE",    "X=y"},
        };
        const auto resp = interpretResponse(true, 200, headers, {});
        QVERIFY(!resp.lastModified.isEmpty());
        QCOMPARE(resp.setCookies.size(), 1);
    }
};

QTEST_MAIN(TstNet)
#include "tst_net.moc"
