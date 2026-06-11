#include "session_controller.h"

#include "player/mpv_backend.h"
#include "player/watchdog.h"
#include "peercast/peercast_url.h"
#include "peercast/peercast_controller.h"
#include "peercast/channel_info.h"
#include "peercast/stream_resolver.h"

#include <QDateTime>
#include <QDebug>
#include <QTimer>

namespace yapcr::app {

SessionController::SessionController(player::MpvBackend* backend, QObject* parent)
    : QObject(parent)
    , backend_(backend)
{
    // BBS セッション（M3.6）: 一度生成し bbsRefresh() で再 init する
    bbs_ = new bbs::BbsSession(this);
    connect(bbs_, &bbs::BbsSession::settingLoaded, this, &SessionController::onBbsSettingLoaded);
    connect(bbs_, &bbs::BbsSession::subjectLoaded, this, &SessionController::onBbsSubjectLoaded);  // M3.9
    connect(bbs_, &bbs::BbsSession::datLoaded,     this, &SessionController::onBbsDatLoaded);
    connect(bbs_, &bbs::BbsSession::postSucceeded, this, &SessionController::onBbsPostSucceeded);
    connect(bbs_, &bbs::BbsSession::postFailed,    this, &SessionController::onBbsPostFailed);
    connect(bbs_, &bbs::BbsSession::loadFailed,    this, &SessionController::onBbsLoadFailed);
}

SessionController::~SessionController() = default;

// ---- 公開 API -------------------------------------------------------------

void SessionController::start(const QString& path,
                               const QString& name,
                               const QString& contact,
                               bool           commandline)
{
    // M3.8: 再 start() 時に前回の BBS ポーリングを止める（二重起動防止）
    stopBbsPolling();

    path_        = path;
    name_        = name;
    contact_     = contact;
    commandline_ = commandline;

    // タイトルを更新する（name が空の場合は path をそのまま表示）
    emit titleChanged(name.isEmpty() ? path : name);

    // 前の PeerCast セッションリソースを破棄する
    teardownPeerCast();

    // /pls/ URL か判定する
    const auto peca = peercast::PeerCastUrl::parse(path);
    if (peca.valid) {
        plsUrl_ = path;
        // 非同期で /stream/ URL を解決してから mpv に load する
        emit statusMessage(tr("PeerCast URL を解決中..."));

        if (resolver_) { resolver_->deleteLater(); }
        resolver_ = new peercast::StreamResolver(this);
        connect(resolver_, &peercast::StreamResolver::resolved,
                this,       &SessionController::onStreamResolved);
        connect(resolver_, &peercast::StreamResolver::failed,
                this,       &SessionController::onStreamFailed);
        resolver_->resolve(path);
    } else {
        // ローカルファイルまたは直 URL — controller/watchdog は起動しない（CLI 互換）
        plsUrl_.clear();
        streamUrl_.clear();
        backend_->load(path);
        emit statusMessage(tr("再生中: %1").arg(path));
    }

    // M3.8: 3引数起動（commandline）かつ contact があれば BBS を自動接続する。
    // peercast / ローカル再生のいずれでも contact があれば実況ポーリングを開始する。
    if (commandline_ && !contact_.isEmpty()) {
        bbsRefresh();
    }

    // M6: B1 — pls URL を UI（クリップボード/ファイルダイアログ）から開いた場合、
    // contact が未指定でも viewxml の url フィールドで BBS を自動接続する。
    // onChannelInfo() が contact を取得し次第 bbsRefresh() を呼ぶ。
    autoConnectBbsOnInfo_ = peca.valid && !commandline_;
}

// ---- 手動操作スロット --------------------------------------------------------

void SessionController::manualBump() {
    if (!controller_) {
        emit statusMessage(tr("PeerCast セッションがありません"));
        return;
    }
    controller_->bump(/*guardByRelay=*/false);
}

void SessionController::manualStop() {
    if (!controller_) {
        emit statusMessage(tr("PeerCast セッションがありません"));
        return;
    }
    controller_->stop(/*guardByRelay=*/false);
}

void SessionController::manualReload() {
    if (streamUrl_.isEmpty()) {
        emit statusMessage(tr("再読込できる URL がありません"));
        return;
    }
    backend_->load(streamUrl_);
    emit statusMessage(tr("再読込: %1").arg(streamUrl_));
}

// ---- ストリーム解決スロット ---------------------------------------------------

void SessionController::onStreamResolved(const QString& streamUrl) {
    qDebug() << "[peercast] stream URL:" << streamUrl;
    streamUrl_ = streamUrl;
    emit statusMessage(tr("接続中: %1").arg(streamUrl));
    backend_->load(streamUrl);

    // PeerCast セッションを構成する（stream URL 取得後に初回情報取得）
    setupPeerCast();
}

void SessionController::onStreamFailed() {
    qDebug() << "[peercast] stream URL 解決失敗";
    emit statusMessage(tr("PeerCast URL の解決に失敗しました"));
}

// ---- PeerCast セットアップ ---------------------------------------------------

void SessionController::setupPeerCast() {
    // 前のリソースが残っていれば破棄（二重呼び出し保護）
    teardownPeerCast();

    const auto peca = peercast::PeerCastUrl::parse(plsUrl_);
    if (!peca.valid) { return; }

    // PeerCastController
    controller_ = new peercast::PeerCastController(peca.host, peca.port, peca.id, this);
    connect(controller_, &peercast::PeerCastController::channelInfo,
            this,        &SessionController::onChannelInfo);
    connect(controller_, &peercast::PeerCastController::infoFailed,
            this,        &SessionController::onInfoFailed);
    connect(controller_, &peercast::PeerCastController::bumped,
            this,        &SessionController::onBumped);
    connect(controller_, &peercast::PeerCastController::stopped,
            this,        &SessionController::onStopped);
    connect(controller_, &peercast::PeerCastController::controlError,
            this,        &SessionController::onControlError);

    // viewxml 定期ポーリングタイマ（30 秒）
    infoTimer_ = new QTimer(this);
    infoTimer_->setInterval(kInfoPollIntervalMs);
    connect(infoTimer_, &QTimer::timeout, controller_,
            &peercast::PeerCastController::requestInfo);
    infoTimer_->start();

    // Watchdog
    watchdog_ = new player::Watchdog(this);
    connect(backend_, &player::MpvBackend::coreIdleChanged,
            this, [this](bool idle) {
                watchdog_->onCoreIdle(idle, QDateTime::currentMSecsSinceEpoch());
            });
    connect(backend_, &player::MpvBackend::cacheTimeChanged,
            this, [this](double t) {
                watchdog_->onCacheTime(t, QDateTime::currentMSecsSinceEpoch());
            });
    connect(backend_, &player::MpvBackend::fileLoaded,
            this, [this] {
                watchdog_->onFileLoaded(QDateTime::currentMSecsSinceEpoch());
            });
    connect(backend_, &player::MpvBackend::endFile,
            this, [this](int reason) {
                watchdog_->onEndFile(reason, QDateTime::currentMSecsSinceEpoch());
            });

    connect(watchdog_, &player::Watchdog::reloadRequested,
            this,      &SessionController::onReloadRequested);
    connect(watchdog_, &player::Watchdog::bumpRequested,
            this,      &SessionController::onBumpRequested);
    connect(watchdog_, &player::Watchdog::supplyLost,
            this,      &SessionController::onSupplyLost);

    // Watchdog 評価タイマ（3 秒）
    watchdogTimer_ = new QTimer(this);
    watchdogTimer_->setInterval(kWatchdogTickMs);
    connect(watchdogTimer_, &QTimer::timeout, this, [this] {
        watchdog_->evaluate(QDateTime::currentMSecsSinceEpoch());
    });
    watchdogTimer_->start();

    // 初回チャンネル情報取得
    controller_->requestInfo();
}

void SessionController::teardownPeerCast() {
    if (infoTimer_) {
        infoTimer_->stop();
        infoTimer_->deleteLater();
        infoTimer_ = nullptr;
    }
    if (watchdogTimer_) {
        watchdogTimer_->stop();
        watchdogTimer_->deleteLater();
        watchdogTimer_ = nullptr;
    }
    if (watchdog_) {
        watchdog_->deleteLater();
        watchdog_ = nullptr;
    }
    if (controller_) {
        controller_->deleteLater();
        controller_ = nullptr;
    }
    // setupPeerCast() で backend_ から this へのラムダ接続を全て切断する。
    // backend_ は SessionController より長生きするため、dangling ラムダを防ぐ。
    // (QObject::disconnect の static 形式: sender から receiver への全接続を解除)
    QObject::disconnect(backend_, nullptr, this, nullptr);
}

// ---- PeerCastController スロット -------------------------------------------

void SessionController::onChannelInfo(const peercast::ChannelInfo& info) {
    const QString title  = peercast::formatTitle(info);
    const QString status = peercast::formatStatus(info);
    if (!title.isEmpty())  { emit titleChanged(title); }
    if (!status.isEmpty()) { emit statusMessage(status); }

    // M6: B1 — pls URL を UI から開いたとき、初回 viewxml 受信で contact を自動設定して BBS 接続する。
    // autoConnectBbsOnInfo_ は start() で1回セットされ、ここで1回だけ消費する（以降は通常 poll）。
    if (autoConnectBbsOnInfo_ && !info.url.isEmpty() && contact_.isEmpty()) {
        autoConnectBbsOnInfo_ = false;
        contact_ = info.url;
        emit statusMessage(tr("BBS を自動接続中: %1").arg(contact_));
        bbsRefresh();
    }
}

void SessionController::onInfoFailed() {
    qDebug() << "[peercast] viewxml 取得失敗";
}

void SessionController::onBumped() {
    emit statusMessage(tr("再接続を要求しました (bump)"));
    // bump 後に stream URL を再 load して新しい上流から受信する
    if (!streamUrl_.isEmpty()) {
        backend_->load(streamUrl_);
    }
}

void SessionController::onStopped() {
    emit statusMessage(tr("配信を停止しました"));
}

void SessionController::onControlError(const QString& reason) {
    qDebug() << "[peercast] controlError:" << reason;
    emit statusMessage(tr("PeerCast エラー: %1").arg(reason));
}

// ---- Watchdog スロット -------------------------------------------------------

void SessionController::onReloadRequested() {
    if (streamUrl_.isEmpty()) { return; }
    qDebug() << "[watchdog→session] reload:" << streamUrl_;
    emit statusMessage(tr("ストリームを再読込しています..."));
    backend_->load(streamUrl_);
}

void SessionController::onBumpRequested() {
    if (!controller_) { return; }
    qDebug() << "[watchdog→session] bump 要求";
    emit statusMessage(tr("上流への再接続を要求しています (auto bump)..."));
    controller_->bump(/*guardByRelay=*/false);
}

void SessionController::onSupplyLost() {
    emit statusMessage(tr("供給が回復しません。手動で操作してください。"));
}

// ---- BBS ポーリング停止ヘルパ（M3.8）----------------------------------------

void SessionController::stopBbsPolling()
{
    if (bbsPollTimer_) {
        bbsPollTimer_->stop();
        // タイマは this 所有のまま（次の bbsRefresh で再利用）
    }
}

// ---- BBS 操作スロット（M3.6）-----------------------------------------------

void SessionController::bbsRefresh()
{
    if (contact_.isEmpty()) {
        emit statusMessage(tr("掲示板 URL が設定されていません"));
        return;
    }
    if (!bbs_->init(contact_)) {
        emit statusMessage(tr("掲示板 URL の解析に失敗しました: %1").arg(contact_));
        return;
    }
    emit statusMessage(tr("BBS を取得中..."));
    // M3.9: 板URL（key 未確定）か スレURL（key 確定済み）で fetch 戦略を分岐する。
    if (bbs_->key().isEmpty()) {
        // cold-start: setting → subject（完了後に onBbsSubjectLoaded で最速スレ選択 → loadDat）
        // stop を先に確定してから selectFastest を呼ぶため setting 先行・dat はここでは呼ばない。
        bbs_->loadSetting();
        // loadSubject は onBbsSettingLoaded 内で呼ぶ（stop 確定後に subject を取得する）
    } else {
        // 従来: setting と dat を並行取得する。setting 失敗（404/403 等）があっても
        // dat 取得・表示を止めないよう分離する。
        bbs_->loadSetting();
        bbs_->loadDat();
    }

    // M3.8: dat ポーリングタイマを（再）起動する。
    // init() のたびにタイマをリセットして間隔ズレを防ぐ。
    if (!bbsPollTimer_) {
        bbsPollTimer_ = new QTimer(this);
        bbsPollTimer_->setInterval(kBbsPollIntervalMs);
        connect(bbsPollTimer_, &QTimer::timeout, this, [this] {
            bbs_->loadDat();  // 条件付き GET — 304 なら merge スキップ
        });
    }
    bbsPollTimer_->start();  // 既に動いていても start() でタイマをリセット
}

// M6: BBS セッションを完全リセット（dat 蓄積・キー等を破棄）して再取得する。
// init() 内で store_.reset() が呼ばれるため先頭レスから読み直す。
// ThreadReset アクション用。UI 側 (MainWindow) は resListPane_->clearRes() を先に呼ぶこと。
void SessionController::bbsReset()
{
    if (contact_.isEmpty()) {
        emit statusMessage(tr("掲示板 URL が設定されていません"));
        return;
    }
    // bbsRefresh() との違い: init() で store を reset → 既読レス蓄積をクリアして最初から読む
    if (!bbs_->init(contact_)) {
        emit statusMessage(tr("掲示板 URL の解析に失敗しました: %1").arg(contact_));
        return;
    }
    emit statusMessage(tr("BBS をリセット中..."));
    bbs_->loadSetting();
    bbs_->loadDat();

    if (bbsPollTimer_) {
        bbsPollTimer_->start();  // タイマをリセット
    }
}

void SessionController::bbsPost(const QString& message)
{
    if (!bbs_->isValid()) {
        emit statusMessage(tr("BBS が初期化されていません（先に「BBS取得/更新」を実行してください）"));
        emit bbsPostFinished(false);
        return;
    }
    // 名前=空、メール=sage フラグで切替（setSage() で外部から制御）。
    const QString mail = sage_ ? QStringLiteral("sage") : QString{};
    bbs_->post(QString{}, mail, message);
}

// ---- BBS シグナル受信スロット（M3.6）----------------------------------------

void SessionController::onBbsSettingLoaded()
{
    // M3.9: 板URL cold-start（key 未確定）のとき、stop 確定後に subject を取得して最速スレを選ぶ。
    // key 確定済みの場合（通常 fetch）は dat と並行でよいため何もしない。
    if (bbs_->key().isEmpty()) {
        bbs_->loadSubject();
    }
}

// M3.9: subjectLoaded → 最速スレ選択 → スレ切替 → loadDat
void SessionController::onBbsSubjectLoaded(const QList<bbs::ThreadInfo>&)
{
    bbs::ThreadInfo out;
    if (!bbs_->selectFastest(out)) {
        emit statusMessage(tr("BBS: 実況中スレが見つかりません（満了済みか対象外）"));
        return;
    }
    if (out.key == bbs_->key()) {
        // 既に最速スレに居る（起動時 key 有りで再 subject した場合）
        return;
    }
    bbs_->change(out.key);
    // タイトル帯更新と ResListPane クリアを促す
    emit bbsThreadChanged(bbsThreadTitle());
    bbs_->loadDat();
    // ポーリングタイマをリセット（新スレに切替直後のズレ防止）
    if (bbsPollTimer_) {
        bbsPollTimer_->start();
    }
}

void SessionController::onBbsDatLoaded(int newCount, bool notModified)
{
    Q_UNUSED(notModified);
    if (newCount > 0) {
        emit statusMessage(tr("BBS 更新: +%1 件").arg(newCount));
    } else {
        emit statusMessage(tr("BBS: 新着なし"));
    }
    emit bbsResAppended(bbs_->dat());
    // M3.7: タイトル帯に最新のスレッドタイトルと件数を通知
    const QString title = bbsThreadTitle();
    emit bbsThreadInfoChanged(title, bbs_->count());
    // M3.9: 満了追従 — stop > 0（実ロード済み）かつ現スレが満了した場合、
    //       最新 subject を取得して最速次スレを選ぶ。onBbsSubjectLoaded が連鎖する。
    // ※ parseSetting は n>0 のみ上書き・既定 1000 なので stop==0 誤発火はないが、
    //   サブプランの明示ゲートを念のため維持する。
    if (bbs_->stop() > 0 && bbs_->isStop()) {
        bbs_->loadSubject();
    }
}

void SessionController::onBbsPostSucceeded()
{
    emit statusMessage(tr("書き込み成功"));
    emit bbsPostFinished(true);
}

void SessionController::onBbsPostFailed(const QString& reason)
{
    emit statusMessage(tr("書き込み失敗: %1").arg(reason));
    emit bbsPostFinished(false);
}

void SessionController::onBbsLoadFailed(bbs::BbsPhase phase, const QString& reason)
{
    const QString phaseStr = [phase]() -> QString {
        switch (phase) {
        case bbs::BbsPhase::Setting: return QStringLiteral("setting");
        case bbs::BbsPhase::Subject: return QStringLiteral("subject");
        case bbs::BbsPhase::Dat:     return QStringLiteral("dat");
        }
        return QStringLiteral("unknown");
    }();
    emit statusMessage(tr("BBS 取得エラー [%1]: %2").arg(phaseStr, reason));
    // M3.9: cold-start 時に setting が失敗しても subject 取得は続行する。
    // キー未確定（board-URL 起動）かつ setting 失敗の場合、stop は既定 1000 のまま loadSubject へ進む。
    if (phase == bbs::BbsPhase::Setting && bbs_ && bbs_->key().isEmpty()) {
        bbs_->loadSubject();
    }
}

// ---- BBS extract クエリ（M3.7）-------------------------------------------------

QList<bbs::ResInfo> SessionController::bbsByRef(int resNumber) const
{
    if (!bbs_ || !bbs_->isValid()) return {};
    return bbs_->store().byRef(resNumber);
}

QList<bbs::ResInfo> SessionController::bbsByRange(bbs::Range range) const
{
    if (!bbs_ || !bbs_->isValid()) return {};
    return bbs_->store().byRange(range);
}

QList<bbs::ResInfo> SessionController::bbsRecent(int n) const
{
    if (!bbs_ || !bbs_->isValid()) return {};
    const int total = bbs_->count();
    if (total <= 0) return {};
    const int first = qMax(1, total - n + 1);
    return bbs_->store().byRange(bbs::Range{first, total});
}

QString SessionController::bbsThreadTitle() const
{
    if (!bbs_ || !bbs_->isValid()) return {};
    const QString t = bbs_->threadTitle();
    return t.isEmpty() ? bbs_->boardTitle() : t;
}

}  // namespace yapcr::app
