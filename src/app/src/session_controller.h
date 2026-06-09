#pragma once

#include "bbs/bbs_session.h"  // BbsSession, BbsPhase, ResInfo（yapcr::bbs）

#include <QList>
#include <QObject>
#include <QString>

class QTimer;

namespace yapcr::player {
class MpvBackend;
class Watchdog;
}

namespace yapcr::peercast {
class StreamResolver;
class PeerCastController;
struct ChannelInfo;
}

namespace yapcr::app {

// OpenFile 相当のオーケストレーション層。
// CLI/設定を受け、peercast URL 解決と player への配線を担う。
//
// M1: /pls/ URL の /stream/ 解決 + MpvBackend への load。
// M2: PeerCastController (bump/stop/viewxml) + Watchdog（停止検知→reload/bump）を追加。
// M3 以降: contact を bbs.init() に渡す。
class SessionController : public QObject {
    Q_OBJECT

public:
    explicit SessionController(player::MpvBackend* backend,
                               QObject* parent = nullptr);
    ~SessionController() override;

    // 再生を開始するオーケストレーションエントリ。
    // path=再生URL/ファイルパス, name=チャンネル名, contact=掲示板URL,
    // commandline=CLI 起動か（contact を自動接続に使うかの判定。M3 で消費）。
    // PCRPlayer MainDlgSub.cpp の OpenFile() に相当。
    void start(const QString& path,
               const QString& name       = {},
               const QString& contact    = {},
               bool           commandline = false);

    // ---- BBS 操作（M3.6） -----

    // contact URL で BbsSession を init し、setting→dat を1回フェッチする。
    // 重複呼び出し時は BbsSession をリセットして再取得する。
    // M3.8: ポーリングタイマ・commandline && contact 自動起動はここに後付けする。
    Q_INVOKABLE void bbsRefresh();

    // BBS に書き込む（名前/メールは空、本文のみの最小構成）。
    // 書き込み中は bbsPostFinished が emit されるまで再呼び出し不可（UI 側が無効化する）。
    Q_INVOKABLE void bbsPost(const QString& message);

    QString currentPath()    const { return path_; }
    QString currentName()    const { return name_; }
    QString currentContact() const { return contact_; }
    bool    isCommandLine()  const { return commandline_; }

public slots:
    // 手動 PeerCast 操作（メニューから呼ぶ）
    void manualBump();
    void manualStop();
    void manualReload();

signals:
    // タイトルバーに表示すべき文字列が変わった。
    void titleChanged(const QString& title);

    // ステータスバーへのメッセージ。
    void statusMessage(const QString& msg);

    // BBS dat が更新された。resList は全レス（差分追記は ResListPane 側で管理）。
    void bbsResAppended(const QList<yapcr::bbs::ResInfo>& resList);

    // 書き込み結果。ok=true: 成功（入力欄をクリアする）、false: 失敗。
    void bbsPostFinished(bool ok);

private slots:
    void onStreamResolved(const QString& streamUrl);
    void onStreamFailed();
    void onChannelInfo(const peercast::ChannelInfo& info);
    void onInfoFailed();
    void onBumped();
    void onStopped();
    void onControlError(const QString& reason);
    void onReloadRequested();
    void onBumpRequested();
    void onSupplyLost();

    // BBS シグナル受信スロット（M3.6）
    void onBbsSettingLoaded();
    void onBbsDatLoaded(int newCount, bool notModified);
    void onBbsPostSucceeded();
    void onBbsPostFailed(const QString& reason);
    void onBbsLoadFailed(yapcr::bbs::BbsPhase phase, const QString& reason);

private:
    // PeerCast セッションのセットアップ（PeerCastController + Watchdog + タイマ生成）
    void setupPeerCast();

    // 既存の PeerCast セッションリソースを破棄する
    void teardownPeerCast();

    // viewxml ポーリング間隔（ms）
    static constexpr int kInfoPollIntervalMs  = 30'000;  // 30 秒
    // Watchdog 評価間隔（ms）
    static constexpr int kWatchdogTickMs      = 3'000;   // 3 秒

    player::MpvBackend*            backend_;
    peercast::StreamResolver*      resolver_{nullptr};
    peercast::PeerCastController*  controller_{nullptr};
    player::Watchdog*              watchdog_{nullptr};
    QTimer*                        infoTimer_{nullptr};
    QTimer*                        watchdogTimer_{nullptr};

    QString path_;
    QString name_;
    QString contact_;
    bool    commandline_{false};

    QString plsUrl_;     // /pls/ URL（PeerCast セッションのとき設定）
    QString streamUrl_;  // /stream/ URL（解決後）

    // BBS セッション（M3.6: 一度生成し init()/loadSetting()/loadDat() を繰り返す）
    bbs::BbsSession* bbs_{nullptr};
};

}  // namespace yapcr::app
