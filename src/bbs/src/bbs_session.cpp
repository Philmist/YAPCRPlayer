#include "bbs/bbs_session.h"

#include "bbs/board_url.h"
#include "bbs/charset.h"
#include "bbs/dat.h"
#include "bbs/post.h"
#include "bbs/setting.h"
#include "bbs/subject.h"
#include "net/http_client.h"
#include "net/http_message.h"

#include <QDateTime>
#include <QUrl>

namespace yapcr::bbs {

// ---- フリー関数への明示的エイリアス（同名メンバ関数との名前衝突を回避） ----
// bbs_session.h が同名メンバ（boardTopUrl/threadTopUrl）を宣言しているため、
// .cpp 内でフリー関数を呼ぶ際は完全修飾名を使う。
namespace detail {
inline QString mkBoardUrl  (const Board& b, BoardType t) { return ::yapcr::bbs::boardUrl  (b, t); }
inline QString mkThreadUrl (const Board& b, const QString& k, BoardType t) { return ::yapcr::bbs::threadUrl(b, k, t); }
inline QString mkSettingUrl(const Board& b, BoardType t) { return ::yapcr::bbs::settingUrl(b, t); }
inline QString mkSubjectUrl(const Board& b, BoardType t) { return ::yapcr::bbs::subjectUrl(b, t); }
inline QString mkDatUrl    (const Board& b, const QString& k, BoardType t) { return ::yapcr::bbs::datUrl    (b, k, t); }
}

// ---------- コンストラクタ / デストラクタ ----------

BbsSession::BbsSession(QObject* parent)
    : QObject(parent)
    , settingClient_(new net::HttpClient(this))
    , subjectClient_(new net::HttpClient(this))
    , datClient_    (new net::HttpClient(this))
    , postClient_   (new net::HttpClient(this))
{
    connect(settingClient_, &net::HttpClient::finished,
            this, &BbsSession::onSettingFinished);
    connect(subjectClient_, &net::HttpClient::finished,
            this, &BbsSession::onSubjectFinished);
    connect(datClient_,     &net::HttpClient::finished,
            this, &BbsSession::onDatFinished);
    connect(postClient_,    &net::HttpClient::finished,
            this, &BbsSession::onPostFinished);
}

BbsSession::~BbsSession() = default;

// ---------- 初期化・切替 ----------

bool BbsSession::init(const QString& contactUrl)
{
    loc_ = parseContactUrl(contactUrl);
    if (!loc_.valid) { return false; }
    store_.reset();
    subject_.clear();
    datLastModified_.clear();
    return true;
}

bool BbsSession::change(const QString& newKey)
{
    if (!loc_.valid) { return false; }
    loc_.thread.key   = newKey;
    loc_.thread.title.clear();
    loc_.thread.count = 0;
    store_.reset();
    datLastModified_.clear();
    return true;
}

// ---------- 非同期 fetch ----------

void BbsSession::loadSetting()
{
    if (!loc_.valid) { return; }
    settingClient_->get(QUrl(detail::mkSettingUrl(loc_.board, loc_.type)));
}

void BbsSession::loadSubject()
{
    if (!loc_.valid) { return; }
    subjectClient_->get(QUrl(detail::mkSubjectUrl(loc_.board, loc_.type)));
}

void BbsSession::loadDat()
{
    if (!loc_.valid || loc_.thread.key.isEmpty()) { return; }

    net::HttpRequest req;
    req.method = net::HttpMethod::Get;
    req.url    = QUrl(detail::mkDatUrl(loc_.board, loc_.thread.key, loc_.type));
    if (!datLastModified_.isEmpty()) {
        req.headers.append({QByteArrayLiteral("If-Modified-Since"),
                            datLastModified_.toUtf8()});
    }
    datClient_->send(req);
}

// ---------- private slots ----------

void BbsSession::onSettingFinished(const net::HttpResponse& resp)
{
    if (!resp.ok || resp.statusCode != 200) {
        emit loadFailed(BbsPhase::Setting,
                        QStringLiteral("HTTP %1").arg(resp.statusCode));
        return;
    }
    const QString text    = decodeFrom(resp.body, loc_.board.code);
    const SettingInfo info = parseSetting(text);
    loc_.board.title   = info.title;
    loc_.board.noname  = info.noname;
    loc_.thread.stop   = info.stop;
    emit settingLoaded();
}

void BbsSession::onSubjectFinished(const net::HttpResponse& resp)
{
    if (!resp.ok || resp.statusCode != 200) {
        emit loadFailed(BbsPhase::Subject,
                        QStringLiteral("HTTP %1").arg(resp.statusCode));
        return;
    }
    const QString text = decodeFrom(resp.body, loc_.board.code);
    subject_ = parseSubject(text, loc_.type);
    emit subjectLoaded(subject_);
}

void BbsSession::onDatFinished(const net::HttpResponse& resp)
{
    if (resp.notModified) {
        emit datLoaded(0, true);
        return;
    }
    if (!resp.ok || resp.statusCode != 200) {
        emit loadFailed(BbsPhase::Dat,
                        QStringLiteral("HTTP %1").arg(resp.statusCode));
        return;
    }
    if (!resp.lastModified.isEmpty()) {
        datLastModified_ = resp.lastModified;
    }
    const QString text         = decodeFrom(resp.body, loc_.board.code);
    const QList<ResInfo> parsed = parseDat(text, loc_.type);
    const int newCount         = store_.merge(parsed, loc_.type);
    emit datLoaded(newCount, false);
}

// ---------- アクセサ ----------

QList<ResInfo> BbsSession::dat() const
{
    return store_.all();
}

QList<ThreadInfo> BbsSession::subject() const
{
    return subject_;
}

const DatStore& BbsSession::store() const
{
    return store_;
}

QString BbsSession::currentUrl() const
{
    if (!loc_.valid) { return {}; }
    if (!loc_.thread.key.isEmpty()) {
        return detail::mkThreadUrl(loc_.board, loc_.thread.key, loc_.type);
    }
    return detail::mkBoardUrl(loc_.board, loc_.type);
}

QString BbsSession::boardTopUrl() const
{
    if (!loc_.valid) { return {}; }
    return detail::mkBoardUrl(loc_.board, loc_.type);
}

QString BbsSession::threadTopUrl() const
{
    if (!loc_.valid || loc_.thread.key.isEmpty()) { return {}; }
    return detail::mkThreadUrl(loc_.board, loc_.thread.key, loc_.type);
}

QString BbsSession::boardTitle()  const { return loc_.board.title; }
QString BbsSession::threadTitle() const { return loc_.thread.title; }
QString BbsSession::noname()      const { return loc_.board.noname; }
QString BbsSession::key()         const { return loc_.thread.key; }
int     BbsSession::stop()        const { return loc_.thread.stop; }
int     BbsSession::count()       const { return store_.count(); }
bool    BbsSession::isStop()      const { return store_.count() >= loc_.thread.stop; }
bool    BbsSession::isValid()     const { return loc_.valid; }

// ---------- 書き込み（M3.5） ----------

void BbsSession::post(const QString& name, const QString& mail, const QString& message)
{
    if (!loc_.valid || loc_.thread.key.isEmpty()) {
        emit postFailed(QStringLiteral("スレッドキーが未設定です"));
        return;
    }
    pendingName_    = name;
    pendingMail_    = mail;
    pendingMessage_ = message;
    pendingTime_    = QDateTime::currentSecsSinceEpoch();
    postAttempt_    = 0;
    sendPost_();
}

void BbsSession::sendPost_(const QList<QNetworkCookie>& extraCookies)
{
    const QByteArray body    = buildPostBody(loc_.board, loc_.thread.key, loc_.type,
                                             pendingName_, pendingMail_, pendingMessage_,
                                             pendingTime_);
    const QByteArray cookie  = buildCookieHeader(pendingName_, pendingMail_,
                                                 loc_.board.code, extraCookies);
    const QByteArray referer = detail::mkThreadUrl(loc_.board, loc_.thread.key, loc_.type)
                               .toUtf8();

    postClient_->post(
        QUrl(writeUrl(loc_.board, loc_.type)),
        body,
        {{QByteArrayLiteral("Referer"), referer},
         {QByteArrayLiteral("Cookie"),  cookie}});
}

void BbsSession::onPostFinished(const net::HttpResponse& resp)
{
    if (!resp.ok) {
        emit postFailed(QStringLiteral("ネットワークエラー"));
        return;
    }
    const QString html = decodeFrom(resp.body, loc_.board.code);
    const WriteClassification cls = classifyWriteResponse(html);
    const PostAction action = nextPostAction(cls.result, postAttempt_,
                                             !resp.setCookies.isEmpty());
    switch (action) {
    case PostAction::Succeed:
        emit postSucceeded();
        break;
    case PostAction::Resend:
        ++postAttempt_;
        sendPost_(resp.setCookies);
        break;
    case PostAction::Fail:
        emit postFailed(cls.message.isEmpty()
                        ? QStringLiteral("書き込みエラー")
                        : cls.message);
        break;
    }
}

}  // namespace yapcr::bbs
