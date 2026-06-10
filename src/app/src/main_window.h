#pragma once

#include <QMainWindow>

#include "display_modes.h"  // M4.1: FitMode

class QAction;
class QActionGroup;
class QKeyEvent;
class QMouseEvent;
class QSplitter;
class QWidget;

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
    void keyPressEvent(QKeyEvent* event) override;                               // M4.3: F=全画面トグル, Esc=全画面解除
    void mouseDoubleClickEvent(QMouseEvent* event) override;                     // M4.3: ダブルクリックで全画面トグル
    bool nativeEvent(const QByteArray& eventType, void* message, qintptr* result) override; // 映像子 HWND クリック検出

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

    // M4.3: 全画面トグル・遷移。
    void toggleFullScreen();  // メニュー/F キー/ダブルクリックの共通エントリ
    void enterFullScreen();   // バー退避 → サイズ固定一時解除 → showFullScreen()
    void leaveFullScreen();   // showNormal() → バー復帰 → サイズモード再適用
    void reapplySizeMode();   // currentSizeMode_ に応じて applyZoom/applyAbsoluteSize/releaseSizeFixed を呼ぶ

    // M4.4: スナップショット保存・フォルダ表示。
    void    takeSnapshot();                // S キー / メニューでスナップショットを保存
    void    openSnapshotFolder();          // 保存フォルダをエクスプローラで開く
    QString snapshotDirectory() const;    // 保存先（Pictures/YAPCRPlayer）。// M5: config化

    VideoHostWidget*         videoWidget_{nullptr};
    player::MpvBackend*      mpv_{nullptr};
    SessionController*       session_{nullptr};
    bool                     attached_{false};

    // M2: 手動操作アクション
    QAction* actBump_{nullptr};
    QAction* actStop_{nullptr};
    QAction* actReload_{nullptr};

    // M3.6: BBS（レス一覧 + 書き込み欄）
    ResListPane*  resListPane_{nullptr};
    ResInputBar*  resInputBar_{nullptr};
    QSplitter*    videoResListSplitter_{nullptr};  // 映像 / レス一覧 分割バー
    bool          bbsUserClosed_{false};    // ユーザーが手動で閉じたら true（自動再表示を抑制）
    QAction*      actBbsRefresh_{nullptr};
    QAction*      actToggleBbs_{nullptr};
    QAction*      actToggleResList_{nullptr};

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

    // M4.3: 全画面
    QAction* actFullScreen_{nullptr};    // チェック可能・全画面状態に同期
    int      lastAbsW_{0};             // 絶対サイズ再適用用（applyAbsoluteSize で記録）
    int      lastAbsH_{0};

    // M4.4: スナップショット
    QAction* actSnapshot_{nullptr};        // 「スナップショット保存」アクション
    QAction* actSnapshotFolder_{nullptr};  // 「保存フォルダを開く」アクション

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
