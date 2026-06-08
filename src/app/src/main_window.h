#pragma once

#include <QMainWindow>

namespace yapcr::player {
class MpvBackend;
}

namespace yapcr::app {

class VideoHostWidget;
class SessionController;

// アプリケーションのメインウィンドウ。
//
// 構成:
//   中央: VideoHostWidget（mpv の --wid 埋め込み先）
//   下部: QStatusBar（ステータスメッセージ表示）
//
// 使い方:
//   show() 後に openMedia() を呼ぶ。
//   show() 時に mpv の attach が自動的に行われる（showEvent で一度だけ）。
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

    // 再生を開始する。show() の前後どちらから呼んでもよい。
    // path=再生URL/ファイルパス, name=チャンネル名, contact=掲示板URL,
    // commandline=CLI 起動か
    void openMedia(const QString& path,
                   const QString& name       = {},
                   const QString& contact    = {},
                   bool           commandline = false);

protected:
    void showEvent(QShowEvent* event) override;

private slots:
    void onTitleChanged(const QString& title);
    void onStatusMessage(const QString& msg);
    void onMpvLogMessage(const QString& prefix, const QString& level, const QString& text);

private:
    // mpv を VideoHostWidget にアタッチする。show() 後に一度だけ呼ぶ。
    void attachMpv();

    VideoHostWidget*         videoWidget_{nullptr};
    player::MpvBackend*      mpv_{nullptr};
    SessionController*       session_{nullptr};
    bool                     attached_{false};

    // openMedia が attach より先に呼ばれた場合に備えて引数を保持する
    struct PendingMedia {
        bool    valid{false};
        QString path;
        QString name;
        QString contact;
        bool    commandline{false};
    } pending_;
};

}  // namespace yapcr::app
