#pragma once

#include <QMainWindow>

#include "display_modes.h"  // M4.1: FitMode

class QAction;
class QActionGroup;
class QDockWidget;

namespace yapcr::player {
class MpvBackend;
}

namespace yapcr::app {

class VideoHostWidget;
class SessionController;
class ResListPane;
class ResInputBar;
class BoardTitleBar;
class ResPopup;

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

    // M4.1: 現在の currentFit_/currentAspectX_/Y_ を mpv に適用する。
    void applyFitMode();

    // M4.2: 映像ウィジェットのサイズ固定・解除。
    void applyZoom(int percent);               // ズーム%をネイティブサイズに掛けて固定
    void applyAbsoluteSize(int w, int h);      // 映像領域をちょうど w×h に固定（バー除外正確ピクセル）
    void releaseSizeFixed();                   // setFixedSize を解除して自由リサイズに戻す

    VideoHostWidget*         videoWidget_{nullptr};
    player::MpvBackend*      mpv_{nullptr};
    SessionController*       session_{nullptr};
    bool                     attached_{false};

    // M2: 手動操作アクション
    QAction* actBump_{nullptr};
    QAction* actStop_{nullptr};
    QAction* actReload_{nullptr};

    // M3.6: BBS ペイン・入力バー・ドック
    ResListPane*  resListPane_{nullptr};
    ResInputBar*  resInputBar_{nullptr};
    QDockWidget*  resDock_{nullptr};
    QDockWidget*  inputDock_{nullptr};
    QAction*      actBbsRefresh_{nullptr};

    // M3.7: 掲示板タイトル帯 + 直近レスポップアップ（映像上オーバーレイ）
    BoardTitleBar* boardTitleBar_{nullptr};
    ResPopup*      recentPopup_{nullptr};

    // M4.1: フィット/アスペクトモード状態
    QActionGroup* displayModeGroup_{nullptr};
    FitMode       currentFit_{FitMode::Inscribe};
    int           currentAspectX_{0};
    int           currentAspectY_{0};

    // M4.2: サイズモード（自由リサイズ / ズーム% / 絶対サイズ）
    enum class SizeMode { Free, Zoom, Absolute };
    QActionGroup* sizeModeGroup_{nullptr};
    SizeMode      currentSizeMode_{SizeMode::Free};
    int           currentZoom_{0};    // 選択中のズーム%（0 = 未選択）
    int           lastVideoW_{0};     // videoSizeChanged で更新されるネイティブ映像幅
    int           lastVideoH_{0};     // videoSizeChanged で更新されるネイティブ映像高さ

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
