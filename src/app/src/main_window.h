#pragma once

#include <QMainWindow>

#include "display_modes.h"   // M4.1: FitMode
#include "config/config.h"   // M5.0: 設定構造体
#include "action_registry.h" // M5.1: 中央アクションディスパッチャ
#include "volume_state.h"    // M5.3: 音量クランプ・最小化連動ミュート状態機械

class QAction;
class QActionGroup;
class QContextMenuEvent;
class QKeyEvent;
class QMenu;
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
    // M5.0: 起動時にロードした Config を受け取る。
    // M5.2: configPath を追加し ReloadConfig / OpenConfigFolder で利用。
    explicit MainWindow(const config::Config& cfg,
                        const QString& configPath,
                        QWidget* parent = nullptr);
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
    void contextMenuEvent(QContextMenuEvent* event) override;                    // M6: 右クリックコンテキストメニュー
    void changeEvent(QEvent* event) override;                                    // M5.3: 最小化/復帰検出（連動ミュート）
    void closeEvent(QCloseEvent* event) override;                               // M5.5: 終了時一括保存
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
    void fixVideoWidgetSize(const QSize& target); // videoWidget_ を target に固定しウィンドウを縮小方向にも追従させる
    void releaseSizeFixed();                   // setFixedSize を解除して自由リサイズに戻す

    // M4.3: 全画面トグル・遷移。
    void toggleFullScreen();  // メニュー/F キー/ダブルクリックの共通エントリ
    void enterFullScreen();   // バー退避 → サイズ固定一時解除 → showFullScreen()
    void leaveFullScreen();   // showNormal() → バー復帰 → サイズモード再適用
    void reapplySizeMode();   // currentSizeMode_ に応じて applyZoom/applyAbsoluteSize/releaseSizeFixed を呼ぶ

    // M4.4 / M6: スナップショット保存・フォルダ表示。
    void    takeSnapshot();                // S キー / メニューでスナップショットを保存
    void    openSnapshotFolder();          // 保存フォルダをエクスプローラで開く
    QString snapshotDirectory() const;    // 保存先（config.snapshot.directory、空なら Pictures/YAPCRPlayer）

    // M6: 右クリックコンテキストメニューを構築する（コンストラクタ末尾で1回呼ぶ）。
    // メニューバー廃止時はここを右クリックの唯一のメニューとする準備。
    void buildContextMenu();

    // M5.3: 音量 / ミュート操作。
    void changeVolume(int delta);          // delta ステップで音量を変更して mpv に適用
    void applyMute();                      // muteState_.effective() を mpv に反映しメニューを同期

    // M6: 一時停止状態をトグルして mpv に反映する。
    void togglePause();

    config::Config           config_;      // M5.0: 起動時ロード済み設定
    QString                  configPath_;  // M5.2: ReloadConfig/OpenConfigFolder 用パス
    bool                     quitStopRequested_{false};  // M5.5: QuitStop アクション起動フラグ
    ActionRegistry           registry_;   // M5.1: 中央アクションディスパッチャ
    int                      currentVolume_{100};  // M5.3: 現在の音量（0-100）
    MuteState                muteState_;           // M5.3: 最小化連動ミュート状態機械
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

    // M5.3: 音量 / ミュート / 最小化
    QAction* actMute_{nullptr};            // 「ミュート」（チェック可・userMute に同期）
    QAction* actMinimizeMute_{nullptr};    // 「最小化時にミュート」（チェック可・config と同期）

    // M6: 再生制御 / 表示トグル
    bool     paused_{false};               // 一時停止状態（cycle pause と同期）
    QAction* actPause_{nullptr};           // 「一時停止」（チェック可）
    bool     isTopmost_{false};            // 最前面フラグ
    QAction* actTopmost_{nullptr};         // 「最前面」（チェック可）
    QAction* actToggleTitle_{nullptr};     // 「メニューバー表示切替」（チェック可）
    QAction* actToggleStatus_{nullptr};    // 「ステータスバー表示切替」（チェック可）
    QAction* actMaximize_{nullptr};        // 「最大化」
    QAction* actSagePost_{nullptr};        // 「sage 投稿」（チェック可）

    // M6: 右クリックコンテキストメニュー（将来のメニューバー廃止への準備）
    QMenu*   contextMenu_{nullptr};

    // M5.1: プリセット QAction ポインタ（keyboard dispatch 後の check state 同期用）
    // sizeModeGroup_: actZoomPresets_ / actSizePresets_ / actFreeSize_
    // displayModeGroup_: actInscribeFit_ / actAspectNone_ / actAspectPresetActions_
    QAction*          actFreeSize_{nullptr};         // 「サイズ固定解除」
    QVector<QAction*> actZoomPresets_;               // Ctrl+1..0 ズーム
    QVector<QAction*> actSizePresets_;               // Shift+1..0 絶対サイズ
    QAction*          actInscribeFit_{nullptr};      // フィット「内接」
    QAction*          actAspectNone_{nullptr};       // アスペクト「なし（内接）」
    QVector<QAction*> actAspectPresetActions_;       // Alt+3..7 アスペクトプリセット

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
