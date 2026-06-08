#pragma once

#include <QObject>
#include <QString>

namespace yapcr::player {
class MpvBackend;
}

namespace yapcr::peercast {
class StreamResolver;
}

namespace yapcr::app {

// OpenFile 相当のオーケストレーション層。
// CLI/設定を受け、peercast URL 解決と player への配線を担う。
//
// M1: /pls/ URL の /stream/ 解決 + MpvBackend への load。
// M2 以降: bump/stop/viewxml/watchdog を追加する。
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

    QString currentPath()    const { return path_; }
    QString currentName()    const { return name_; }
    QString currentContact() const { return contact_; }
    bool    isCommandLine()  const { return commandline_; }

signals:
    // タイトルバーに表示すべき文字列が変わった。
    void titleChanged(const QString& title);

    // ステータスバーへのメッセージ。
    void statusMessage(const QString& msg);

private slots:
    void onStreamResolved(const QString& streamUrl);
    void onStreamFailed();

private:
    player::MpvBackend*       backend_;
    peercast::StreamResolver* resolver_{nullptr};

    QString path_;
    QString name_;
    QString contact_;
    bool    commandline_{false};
};

}  // namespace yapcr::app
