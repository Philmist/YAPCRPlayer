#pragma once

#include "bbs/board_url.h"
#include "bbs/dat_store.h"
#include "bbs/models.h"
#include "bbs/post.h"
#include "net/http_message.h"

#include <QList>
#include <QObject>
#include <QString>

namespace yapcr::net {
class HttpClient;
}

namespace yapcr::bbs {

// fetch フェーズ識別（loadFailed シグナル用）
enum class BbsPhase {
    Setting,
    Subject,
    Dat,
};

// BBSManager 相当を Qt QObject として再設計した BBS fetch オーケストレータ。
// 横断決定#2: 手動スレッドなし / mutex なし / シグナル駆動。
// fetch 連鎖（setting→subject→dat）とポーリングは上位（M3.8 SessionController）の責務。
// テスト戦略: DatStore の純コアをユニットテスト。QObject ネットワーク経路は M3.8 実機確認。
// 移植元: BBSManager（PCRPlayer/PCRPlayer/BBSManager.cpp）
class BbsSession : public QObject {
    Q_OBJECT

public:
    explicit BbsSession(QObject* parent = nullptr);
    ~BbsSession() override;

    // contact URL を解析して内部状態を初期化する（同期）。
    // 無効 URL の場合 false を返す。各 load 前に必ず呼ぶこと。
    // 移植元: BBSManager::init(@20)
    bool init(const QString& contactUrl);

    // スレッドを切り替える（key: 新しいスレッドキー）。
    // DatStore をリセットし、条件付き GET の Last-Modified もクリアする。
    // 移植元: BBSManager::change(@62)
    bool change(const QString& key);

    // ---- 非同期 fetch（完了時に対応シグナルを emit） ----

    // setting.txt を取得してパースし、Board の title/noname と Thread の stop を反映する。
    // 移植元: BBSManager::loadSetting(@77)
    void loadSetting();

    // subject.txt を取得してパースし、subject_ に格納する。
    // 移植元: BBSManager::loadSubject(@94)
    void loadSubject();

    // dat を条件付き GET で取得し、DatStore::merge で集計する。
    // If-Modified-Since を送り 304 の場合は datLoaded(0, true) を emit する（merge スキップ）。
    // M3.x: Range ヘッダによる差分バイト取得（partial()@519）は未実装。full-GET + count ベース dedup で代替。
    // 移植元: BBSManager::loadDat(@115)
    void loadDat();

    // ---- アクセサ ----

    // del 除外の全レス（DatStore::all() 相当）
    QList<ResInfo>    dat()     const;
    QList<ThreadInfo> subject() const;
    // extract 系クエリ（M3.7 hover が byRange/byId/byRef で使用）
    const DatStore&   store()   const;

    // ---- URL/情報 getter（移植元 getURL@576 ほか） ----

    // thread key があればスレッド URL、なければ板 URL を返す（移植元 getURL@576）
    QString currentUrl()  const;
    QString boardTopUrl() const;
    QString threadTopUrl() const;
    QString boardTitle()  const;
    QString threadTitle() const;
    QString noname()      const;
    QString key()         const;
    int     stop()        const;   // Thread::stop（setting.txt BBS_THREAD_STOP）
    int     count()       const;   // store_.count()
    bool    isStop()      const;   // store_.count() >= loc_.thread.stop
    bool    isValid()     const;   // init 済みかつ URL 解析成功

    // ---- 書き込み（非同期 POST）----

    // 書き込みリクエストを発行する。2ch 互換掲示板では確認ページが返る場合があり、
    // その際は Cookie を受領して自動的に再送する（高々 1 回）。
    // 完了時に postSucceeded() または postFailed(reason) を emit する。
    // 移植元: BaseBBS::post()@337
    void post(const QString& name, const QString& mail, const QString& message);

signals:
    void settingLoaded();
    void subjectLoaded(const QList<yapcr::bbs::ThreadInfo>& threads);
    // newCount: 追加レス数 / notModified: 304 応答で merge を行わなかった場合 true
    void datLoaded(int newCount, bool notModified);
    // phase: どの fetch が失敗したか / reason: HTTP ステータス等の説明
    void loadFailed(yapcr::bbs::BbsPhase phase, const QString& reason);
    // 書き込み成功
    void postSucceeded();
    // 書き込み失敗。reason: classifyWriteResponse の message またはネットワークエラー
    void postFailed(const QString& reason);

private slots:
    void onSettingFinished(const yapcr::net::HttpResponse& resp);
    void onSubjectFinished(const yapcr::net::HttpResponse& resp);
    void onDatFinished(const yapcr::net::HttpResponse& resp);
    void onPostFinished(const yapcr::net::HttpResponse& resp);

private:
    BoardLocation     loc_;
    DatStore          store_;
    QList<ThreadInfo> subject_;
    QString           datLastModified_;  // 次回 If-Modified-Since 用（条件付き GET）

    // 用途別 HttpClient（各クライアントは同時 1 リクエストのみ制約 → 別インスタンスで回避）
    net::HttpClient*  settingClient_;
    net::HttpClient*  subjectClient_;
    net::HttpClient*  datClient_;
    net::HttpClient*  postClient_;   // M3.5: 書き込み用（再送で同一インスタンス再利用）

    // 書き込み中間状態（1 回目→再送で保持）
    QString pendingName_;
    QString pendingMail_;
    QString pendingMessage_;
    qint64  pendingTime_{0};     // 1 回目で確定し再送でも同一 time を使う
    int     postAttempt_{0};

    // 内部: POST を実際に発行するヘルパー。extraCookies は再送時に渡す Set-Cookie
    void sendPost_(const QList<QNetworkCookie>& extraCookies = {});
};

}  // namespace yapcr::bbs
