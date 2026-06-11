#include "main_window.h"

#include "video_host_widget.h"
#include "session_controller.h"
#include "res_list_pane.h"   // bbs/models.h を推移的に含む（yapcr::bbs::ResInfo）
#include "res_input_bar.h"
#include "board_title_bar.h" // M3.7: 掲示板タイトル帯
#include "res_popup.h"       // M3.7: hover レス・ポップアップ
#include "bbs/models.h"
#include "player/mpv_backend.h"
#include "common/version.h"
#include "window_geometry.h"   // M4.2: videoTargetForZoom / zoomPresets / sizePresets
#include "snapshot_filename.h" // M4.4: snapshotFilename
#include "action_id.h"         // M5.1: ActionId / defaultKeyMap
#include "shortcut_keys.h"     // M5.1: keyChordFromEvent
#include "media_source.h"      // M5.4: クリップボード/パスの再生ソース種別判定
#include "restore_state.h"    // M5.5: [restore] × [state] 復元値選択（純ロジック）

#include <QAction>
#include <QActionGroup>
#include <QClipboard>
#include <QCursor>
#include <QDateTime>
#include <QDebug>
#include <QDesktopServices>
#include <QDir>
#include <QEvent>
#include <QFileDialog>
#include <QFileInfo>
#include <QGuiApplication>
#include <QKeyEvent>
#include <QMenu>
#include <QMenuBar>
#include <QMouseEvent>
#include <QCloseEvent>
#include <QScreen>
#include <QShowEvent>
#include <QStandardPaths>
#include <QStatusBar>
#include <QString>
#include <QUrl>
#include <QSplitter>
#include <QVBoxLayout>
#include <QWindowStateChangeEvent>
#include <QtGlobal>
#ifdef Q_OS_WIN
#include <windows.h>
#endif

namespace yapcr::app {

MainWindow::MainWindow(const config::Config& cfg,
                       const QString& configPath,
                       QWidget* parent)
    : QMainWindow(parent)
    , config_(cfg)
    , configPath_(configPath)
{
    // M3.7: centralWidget を QWidget コンテナ化し、各ウィジェットを縦積みする。
    // 注意: videoWidget_->winId() は attachMpv()（showEvent 後）で mpv に渡す。
    //       コンテナ化は attach 前に確定させ、attach 後に reparent しないこと。
    {
        QWidget* central = new QWidget(this);
        QVBoxLayout* vl  = new QVBoxLayout(central);
        vl->setContentsMargins(0, 0, 0, 0);
        vl->setSpacing(0);

        // 映像 / レス一覧 をスプリッターで縦に並べる（ドラッグで高さ調整可能）
        videoResListSplitter_ = new QSplitter(Qt::Vertical, central);
        videoResListSplitter_->setChildrenCollapsible(false);

        videoWidget_ = new VideoHostWidget(videoResListSplitter_);
        videoWidget_->setMinimumSize(640, 360);
        videoResListSplitter_->addWidget(videoWidget_);

        resListPane_ = new ResListPane(videoResListSplitter_);
        resListPane_->setMinimumHeight(80);
        resListPane_->hide();  // デフォルト非表示（メニュー「レス一覧 表示切替」で on/off）
        videoResListSplitter_->addWidget(resListPane_);
        videoResListSplitter_->setStretchFactor(0, 3);  // 映像:レス一覧 = 3:1（初期比率）
        videoResListSplitter_->setStretchFactor(1, 1);

        vl->addWidget(videoResListSplitter_, 1);

        boardTitleBar_ = new BoardTitleBar(central);
        vl->addWidget(boardTitleBar_, 0);

        // 書き込み欄（BBS パネル表示切替で on/off、初期は非表示）
        resInputBar_ = new ResInputBar(central);
        resInputBar_->hide();
        vl->addWidget(resInputBar_, 0);

        setCentralWidget(central);
    }

    // ステータスバー（mpv ログ・接続状態の表示）
    statusBar()->showMessage(
        QStringLiteral("%1 %2")
            .arg(QString::fromLatin1(common::appName()))
            .arg(QString::fromLatin1(common::appVersion())));

    // mpv バックエンド（attach は showEvent で行う）
    mpv_ = new player::MpvBackend(this);
    connect(mpv_, &player::MpvBackend::fileLoaded, this, [this] {
        statusBar()->showMessage(tr("再生開始"));
        // M4.1: 再生開始後に選択中のフィットモードを確実に適用する
        //       （再生前に選択した場合、mpv init 後に改めて流す必要がある）
        applyFitMode();
    });
    connect(mpv_, &player::MpvBackend::endFile, this, [this](int reason) {
        // reason: 0=EOF, 1=stop, 2=quit, 3=error
        if (reason == 3) {
            statusBar()->showMessage(tr("再生エラー"));
        } else if (reason == 0) {
            statusBar()->showMessage(tr("再生終了 (EOF)"));
        }
    });
    connect(mpv_, &player::MpvBackend::logMessage,
            this, &MainWindow::onMpvLogMessage);

    // M4.2: 映像ネイティブサイズを保持し、ズーム%選択中は変化に追従して再適用する。
    connect(mpv_, &player::MpvBackend::videoSizeChanged, this, [this](int w, int h) {
        qDebug() << "[mpv] videoSizeChanged:" << w << "x" << h;
        lastVideoW_ = w;
        lastVideoH_ = h;
        if (currentSizeMode_ == SizeMode::Zoom && currentZoom_ > 0) {
            applyZoom(currentZoom_);
        }
    });

    // M4.1: フィットモードの初期値を内接（Inscribe）に設定する。
    //       ここで applyFitMode() を呼んでも mpv が未 init のため no-op。
    //       実際の適用は fileLoaded シグナル受信後に行う。
    currentFit_ = FitMode::Inscribe;

    // セッションコントローラ
    session_ = new SessionController(mpv_, this);
    connect(session_, &SessionController::titleChanged,
            this,     &MainWindow::onTitleChanged);
    connect(session_, &SessionController::statusMessage,
            this,     &MainWindow::onStatusMessage);

    // M3.6: BBS シグナル配線
    connect(session_, &SessionController::bbsResAppended,
            this, [this](const QList<yapcr::bbs::ResInfo>& resList) {
                resListPane_->appendResList(resList);
                // 初回受信時のみ書き込み欄を自動表示。ユーザーが手動で閉じた後は再表示しない。
                if (!resList.isEmpty() && !bbsUserClosed_) {
                    resInputBar_->show();
                }
            });

    // M3.7: hover ポップアップ用クエリプロバイダ注入（右ドック: アンカーポップアップ副次機能）
    resListPane_->setByRefProvider([this](int n) {
        return session_->bbsByRef(n);
    });
    resListPane_->setByRangeProvider([this](yapcr::bbs::Range r) {
        return session_->bbsByRange(r);
    });

    // M3.7: 掲示板タイトル帯 → 直近レスオーバーレイ（本命機能）
    recentPopup_ = new ResPopup(this);
    // スレッドタイトル/件数が更新されたらタイトル帯に反映する
    connect(session_, &SessionController::bbsThreadInfoChanged,
            boardTitleBar_, &BoardTitleBar::setInfo);
    // タイトル帯に hover → 直近 40 件を取得して映像上にポップアップ表示
    connect(boardTitleBar_, &BoardTitleBar::hovered,
            this, [this](QPoint globalPos) {
                const auto recentRes = session_->bbsRecent(40);
                if (!recentRes.isEmpty()) {
                    recentPopup_->showRecent(recentRes, globalPos);
                }
            });
    // タイトル帯上のホイール → ポップアップの遡行へ転送（カーソルは帯に置いたまま）
    connect(boardTitleBar_, &BoardTitleBar::scrolled,
            this, [this](int delta) {
                recentPopup_->scrollRecent(delta);
            });
    // タイトル帯からカーソルが外れたとき、ポップアップ自身の上にいる場合は閉じない
    connect(boardTitleBar_, &BoardTitleBar::left,
            this, [this] {
                const QRect popupRect = recentPopup_->geometry();
                if (!popupRect.contains(QCursor::pos())) {
                    recentPopup_->hidePopup();
                }
            });
    // M3.9: スレッド自動切替時にレス一覧をクリアする（差分追記の破損防止）
    connect(session_, &SessionController::bbsThreadChanged,
            this, [this](const QString&) {
                resListPane_->clearRes();
                recentPopup_->hidePopup();
            });

    connect(session_, &SessionController::bbsPostFinished,
            this, [this](bool ok) {
                resInputBar_->setInputEnabled(true);
                if (ok) { resInputBar_->clearInput(); }
            });
    // 送信時に入力欄を無効化してから SessionController に渡す
    connect(resInputBar_, &ResInputBar::postRequested,
            this, [this](const QString& msg) {
                resInputBar_->setInputEnabled(false);
                session_->bbsPost(msg);
            });

    // M5.5: 音量 / ミュート / ウィンドウジオメトリ の起動時復元 ————————————————————
    // [restore] トグルが true のものだけ [state] 値を適用（純ロジックは restore_state.h）。
    currentVolume_ = restoredVolume(config_.restore, config_.state);
    muteState_.setUser(restoredMute(config_.restore, config_.state));

    // M5.1/M5.2: アクションレジストリ設定 ——————————————————————————————————————————
    // TOML [shortcuts] の差分をデフォルト表に上書きしてキーマップを構築する。
    registry_.setKeyMap(mergeShortcuts(config_.shortcuts));

    // 実装済みアクションのハンドラを登録（未実装アクションはキーマップに載るが
    // ハンドラ未登録のまま → dispatch() が false を返しキーを素通しする）。
    registry_.on(ActionId::Bump,    [this]{ session_->manualBump(); });
    registry_.on(ActionId::Stop,    [this]{ session_->manualStop(); });
    registry_.on(ActionId::Rebuild, [this]{ session_->manualReload(); });
    registry_.on(ActionId::ThreadRefresh, [this]{
        resListPane_->clearRes();
        session_->bbsRefresh();
    });
    registry_.on(ActionId::ToggleBbs, [this]{
        if (resInputBar_->isVisible()) {
            resInputBar_->hide();
            resListPane_->hide();
            bbsUserClosed_ = true;
        } else {
            resInputBar_->show();
            bbsUserClosed_ = false;
        }
    });
    registry_.on(ActionId::ToggleResList, [this]{
        resListPane_->setVisible(!resListPane_->isVisible());
    });
    registry_.on(ActionId::FullScreen,    [this]{ toggleFullScreen(); });
    registry_.on(ActionId::SnapshotSave,  [this]{ takeSnapshot(); });
    registry_.on(ActionId::SnapshotFolder,[this]{ openSnapshotFolder(); });

    // M5.2: 設定再読み込み・フォルダを開く
    registry_.on(ActionId::ReloadConfig, [this]{
        config_ = config::load(configPath_);
        registry_.setKeyMap(mergeShortcuts(config_.shortcuts));
        statusBar()->showMessage(tr("設定を再読み込みしました"), 3000);
    });
    registry_.on(ActionId::OpenConfigFolder, [this]{
        const QString dir = QFileInfo(configPath_).absolutePath();
        QDesktopServices::openUrl(QUrl::fromLocalFile(dir));
    });

    // M5.3: 音量 / ミュート / 最小化 ——————————————————————————————————————————————
    // 音量6種: 通常(±step)・小(±low)・大(±high) の上下
    {
        const int step  = config_.playback.volume_step;
        const int low   = config_.playback.volume_step_low;
        const int high  = config_.playback.volume_step_high;
        registry_.on(ActionId::VolumeUp,       [this, step]{ changeVolume(+step); });
        registry_.on(ActionId::VolumeDown,     [this, step]{ changeVolume(-step); });
        registry_.on(ActionId::VolumeUpLow,    [this, low] { changeVolume(+low);  });
        registry_.on(ActionId::VolumeDownLow,  [this, low] { changeVolume(-low);  });
        registry_.on(ActionId::VolumeUpHigh,   [this, high]{ changeVolume(+high); });
        registry_.on(ActionId::VolumeDownHigh, [this, high]{ changeVolume(-high); });
    }
    // Mute: userMute をトグルして mpv に反映
    registry_.on(ActionId::Mute, [this]{
        muteState_.toggleUser();
        applyMute();
        statusBar()->showMessage(
            muteState_.userMute() ? tr("ミュート") : tr("ミュート解除"), 2000);
    });
    // Minimize: 最小化のみ（自動ミュートは changeEvent で一元処理）
    registry_.on(ActionId::Minimize, [this]{
        showMinimized();
    });
    // MinimizeMute: minimize_mute 設定をランタイムでトグル
    registry_.on(ActionId::MinimizeMute, [this]{
        config_.playback.minimize_mute = !config_.playback.minimize_mute;
        if (actMinimizeMute_) {
            actMinimizeMute_->setChecked(config_.playback.minimize_mute);
        }
        statusBar()->showMessage(
            config_.playback.minimize_mute
                ? tr("最小化時ミュート: ON") : tr("最小化時ミュート: OFF"),
            2000);
    });

    // M5.4: ファイル/URL を開く ＋ クリップボード連携 ——————————————————————————————————
    registry_.on(ActionId::OpenFileDialog, [this]{
        const QString file = QFileDialog::getOpenFileName(
            this, tr("ファイルを開く"), QString(),
            tr("メディアファイル (*.mp4 *.mkv *.avi *.ts *.webm *.mov *.wmv *.flv "
               "*.mp3 *.aac *.flac *.wav);;すべてのファイル (*.*)"));
        if (!file.isEmpty()) { openMedia(file, {}, {}, /*commandline=*/false); }
    });
    registry_.on(ActionId::OpenFromClipboard, [this]{
        const QString text = QGuiApplication::clipboard()->text().trimmed();
        if (classifyMediaSource(text) == MediaSourceKind::Invalid) {
            statusBar()->showMessage(tr("クリップボードに有効なURL/パスがありません"), 3000);
            return;
        }
        openMedia(text, {}, {}, /*commandline=*/false); // BBS 自動接続しない（commandline=false）
    });
    registry_.on(ActionId::CopyPathToClipboard, [this]{
        const QString p = session_->currentPath();
        if (p.isEmpty()) {
            statusBar()->showMessage(tr("コピーする再生URL/パスがありません"), 3000);
            return;
        }
        QGuiApplication::clipboard()->setText(p);
        statusBar()->showMessage(tr("再生URL/パスをコピーしました"), 2000);
    });
    registry_.on(ActionId::CopyContactToClipboard, [this]{
        const QString c = session_->currentContact();
        if (c.isEmpty()) {
            statusBar()->showMessage(tr("コピーする掲示板URLがありません"), 3000);
            return;
        }
        QGuiApplication::clipboard()->setText(c);
        statusBar()->showMessage(tr("掲示板URLをコピーしました"), 2000);
    });
    registry_.on(ActionId::OpenContactInBrowser, [this]{
        const QString c = session_->currentContact();
        if (c.isEmpty()) {
            statusBar()->showMessage(tr("開く掲示板URLがありません"), 3000);
            return;
        }
        QDesktopServices::openUrl(QUrl(c));
    });

    // M5.5: 終了 / 切断して終了 —————————————————————————————————————————————————
    // Quit: 単純終了（closeEvent で [state] 保存）
    // QuitStop: quitStopRequested_ フラグを立ててから close → closeEvent でチャンネル切断
    registry_.on(ActionId::Quit,     [this]{ close(); });
    registry_.on(ActionId::QuitStop, [this]{ quitStopRequested_ = true; close(); });

    // アスペクトプリセットハンドラ（メニュー構築前に登録。ポインタは [this] 経由で実行時参照）
    registry_.on(ActionId::AspectDefault, [this]{
        currentFit_ = FitMode::Inscribe; currentAspectX_ = 0; currentAspectY_ = 0;
        applyFitMode();
        if (actInscribeFit_) { actInscribeFit_->setChecked(true); }
    });
    registry_.on(ActionId::AspectNone, [this]{
        currentFit_ = FitMode::Inscribe; currentAspectX_ = 0; currentAspectY_ = 0;
        applyFitMode();
        if (actAspectNone_) { actAspectNone_->setChecked(true); }
    });
    {
        const auto presets = aspectPresets();
        for (int i = 0; i < presets.size() && i < 5; ++i) {
            const ActionId aid = ActionId(int(ActionId::AspectPreset1) + i);
            const int ax = presets[i].x, ay = presets[i].y;
            registry_.on(aid, [this, ax, ay, i]{
                currentFit_     = FitMode::AspectOverride;
                currentAspectX_ = ax;
                currentAspectY_ = ay;
                applyFitMode();
                if (i < actAspectPresetActions_.size()) {
                    actAspectPresetActions_[i]->setChecked(true);
                }
            });
        }
    }

    // M5.4: 「ファイル」メニュー（最左。ファイル/URL を開く・コピー・ブラウザ） ————————
    {
        QMenu* fileMenu = menuBar()->addMenu(tr("ファイル(&F)"));

        QAction* actOpen = fileMenu->addAction(tr("開く...(&O)"));
        connect(actOpen, &QAction::triggered, this,
                [this]{ registry_.trigger(ActionId::OpenFileDialog); });

        QAction* actOpenClip = fileMenu->addAction(tr("クリップボードから開く(&P)\tCtrl+V"));
        connect(actOpenClip, &QAction::triggered, this,
                [this]{ registry_.trigger(ActionId::OpenFromClipboard); });

        fileMenu->addSeparator();

        QAction* actCopyPath = fileMenu->addAction(tr("再生URL/パスをコピー(&C)\tCtrl+C"));
        connect(actCopyPath, &QAction::triggered, this,
                [this]{ registry_.trigger(ActionId::CopyPathToClipboard); });

        QAction* actCopyContact = fileMenu->addAction(tr("掲示板URLをコピー(&D)"));
        connect(actCopyContact, &QAction::triggered, this,
                [this]{ registry_.trigger(ActionId::CopyContactToClipboard); });

        QAction* actOpenBrowser = fileMenu->addAction(tr("掲示板をブラウザで開く(&B)"));
        connect(actOpenBrowser, &QAction::triggered, this,
                [this]{ registry_.trigger(ActionId::OpenContactInBrowser); });

        // M5.5: 終了導線
        fileMenu->addSeparator();
        QAction* actQuit = fileMenu->addAction(tr("終了(&X)"));
        connect(actQuit, &QAction::triggered, this,
                [this]{ registry_.trigger(ActionId::Quit); });
        QAction* actQuitStop = fileMenu->addAction(tr("切断して終了\tAlt+X"));
        connect(actQuitStop, &QAction::triggered, this,
                [this]{ registry_.trigger(ActionId::QuitStop); });
    }

    // M2: 最小メニューバー（再接続/切断/再読込） ——————————————————————————————————
    {
        QMenu* menu = menuBar()->addMenu(tr("操作(&O)"));

        actBump_   = menu->addAction(tr("再接続 (&B)ump\tAlt+B"));
        actStop_   = menu->addAction(tr("切断 (&Z)top\tAlt+Z"));
        menu->addSeparator();
        actReload_ = menu->addAction(tr("再読込 (&R)eload\tCtrl+R"));

        connect(actBump_,   &QAction::triggered, this, [this]{ registry_.trigger(ActionId::Bump); });
        connect(actStop_,   &QAction::triggered, this, [this]{ registry_.trigger(ActionId::Stop); });
        connect(actReload_, &QAction::triggered, this, [this]{ registry_.trigger(ActionId::Rebuild); });

        // M3.6: BBS 取得/更新・パネル表示切替
        menu->addSeparator();
        actBbsRefresh_ = menu->addAction(tr("BBS 取得/更新 (&G)"));
        connect(actBbsRefresh_, &QAction::triggered, this,
                [this]{ registry_.trigger(ActionId::ThreadRefresh); });
        menu->addSeparator();
        actToggleBbs_ = menu->addAction(tr("BBS パネル (&C) 表示切替\tC"));
        connect(actToggleBbs_, &QAction::triggered, this,
                [this]{ registry_.trigger(ActionId::ToggleBbs); });
        actToggleResList_ = menu->addAction(tr("レス一覧 (&L) 表示切替"));
        connect(actToggleResList_, &QAction::triggered, this,
                [this]{ registry_.trigger(ActionId::ToggleResList); });

        // M5.3: ミュート / 最小化 / 最小化時ミュート
        menu->addSeparator();
        actMute_ = menu->addAction(tr("ミュート (&M)\tM"));
        actMute_->setCheckable(true);
        actMute_->setChecked(muteState_.userMute());
        connect(actMute_, &QAction::triggered, this,
                [this]{ registry_.trigger(ActionId::Mute); });

        QAction* actMinimize = menu->addAction(tr("最小化"));
        connect(actMinimize, &QAction::triggered, this,
                [this]{ registry_.trigger(ActionId::Minimize); });

        actMinimizeMute_ = menu->addAction(tr("最小化時にミュート"));
        actMinimizeMute_->setCheckable(true);
        actMinimizeMute_->setChecked(config_.playback.minimize_mute);
        connect(actMinimizeMute_, &QAction::triggered, this,
                [this]{ registry_.trigger(ActionId::MinimizeMute); });
    }

    // M4.1: 「表示」メニュー（フィットモード・アスペクト比）
    {
        QMenu* viewMenu = menuBar()->addMenu(tr("表示(&V)"));

        // フィットモードとアスペクト比は同一の排他グループとする。
        displayModeGroup_ = new QActionGroup(this);
        displayModeGroup_->setExclusive(true);

        // ---- フィットサブメニュー ----
        QMenu* fitMenu = viewMenu->addMenu(tr("フィット(&F)"));

        auto addFit = [&](const QString& label, FitMode mode, int ax = 0, int ay = 0) {
            QAction* act = fitMenu->addAction(label);
            act->setCheckable(true);
            displayModeGroup_->addAction(act);
            // フィット切替は keyboard shortcut がないため直接 connect
            connect(act, &QAction::triggered, this, [this, mode, ax, ay] {
                currentFit_     = mode;
                currentAspectX_ = ax;
                currentAspectY_ = ay;
                applyFitMode();
            });
            return act;
        };

        actInscribeFit_ = addFit(tr("内接 (&I)nscribe（既定）"), FitMode::Inscribe);
        actInscribeFit_->setChecked(true);  // 初期選択
        addFit(tr("引き伸ばし (&S)tretch"), FitMode::Stretch);
        addFit(tr("充填 (&F)ill"),          FitMode::Fill);
        addFit(tr("等倍 (&U)nscaled"),      FitMode::Unscaled);

        // ---- アスペクト比サブメニュー ----
        QMenu* aspectMenu = viewMenu->addMenu(tr("アスペクト比(&A)"));

        // 「なし（内接）」= 内接相当（Alt+2）
        actAspectNone_ = aspectMenu->addAction(tr("なし（内接）\tAlt+2"));
        actAspectNone_->setCheckable(true);
        displayModeGroup_->addAction(actAspectNone_);
        connect(actAspectNone_, &QAction::triggered, this,
                [this]{ registry_.trigger(ActionId::AspectNone); });

        // プリセット（ハードコード既定。M5.2: config 化）
        const auto aspectList = aspectPresets();
        for (int i = 0; i < aspectList.size() && i < 5; ++i) {
            const ActionId aid = ActionId(int(ActionId::AspectPreset1) + i);
            const QString keyHint = QStringLiteral("\tAlt+%1").arg(3 + i);
            QAction* act = aspectMenu->addAction(
                QString::fromLatin1(aspectList[i].label) + keyHint);
            act->setCheckable(true);
            displayModeGroup_->addAction(act);
            actAspectPresetActions_ << act;
            connect(act, &QAction::triggered, this, [this, aid]{ registry_.trigger(aid); });
        }

        viewMenu->addSeparator();

        // ---- M4.2: サイズ固定（ズーム% / 絶対サイズ）----
        // フィット/アスペクト（displayModeGroup_）とは別軸の排他グループ。
        sizeModeGroup_ = new QActionGroup(this);
        sizeModeGroup_->setExclusive(true);

        // 「自由リサイズ（固定解除）」— 初期選択
        {
            actFreeSize_ = viewMenu->addAction(tr("サイズ固定を解除（自由リサイズ）"));
            actFreeSize_->setCheckable(true);
            actFreeSize_->setChecked(true);
            sizeModeGroup_->addAction(actFreeSize_);
            connect(actFreeSize_, &QAction::triggered, this, [this]{
                currentSizeMode_ = SizeMode::Free;
                currentZoom_     = 0;
                releaseSizeFixed();
            });
        }

        // 「ズーム%」サブメニュー（Ctrl+1..0 に対応）
        {
            QMenu* zoomMenu = viewMenu->addMenu(tr("ズーム(&Z)"));
            const QList<int> zooms = zoomPresets();
            for (int i = 0; i < zooms.size() && i < 10; ++i) {
                const ActionId aid    = ActionId(int(ActionId::ZoomPreset1) + i);
                const int      pct    = zooms[i];
                const QString  keyHint = (i < 9)
                    ? QStringLiteral("\tCtrl+%1").arg(i + 1)
                    : QStringLiteral("\tCtrl+0");
                QAction* act = zoomMenu->addAction(
                    QStringLiteral("%1%").arg(pct) + keyHint);
                act->setCheckable(true);
                sizeModeGroup_->addAction(act);
                actZoomPresets_ << act;

                // ハンドラ（keyboard shortcut も menu click も同じ処理）
                registry_.on(aid, [this, pct, act]{
                    currentSizeMode_ = SizeMode::Zoom;
                    currentZoom_     = pct;
                    applyZoom(pct);
                    act->setChecked(true);  // QActionGroup が他を自動解除
                });
                connect(act, &QAction::triggered, this, [this, aid]{ registry_.trigger(aid); });
            }
        }

        // 「絶対サイズ」サブメニュー（Shift+1..0 に対応）
        {
            QMenu* sizeMenu = viewMenu->addMenu(tr("絶対サイズ(&S)"));
            const auto sizes = sizePresets();
            for (int i = 0; i < sizes.size() && i < 10; ++i) {
                const ActionId aid    = ActionId(int(ActionId::SizePreset1) + i);
                const int      pw     = sizes[i].w;
                const int      ph     = sizes[i].h;
                const QString  keyHint = (i < 9)
                    ? QStringLiteral("\tShift+%1").arg(i + 1)
                    : QStringLiteral("\tShift+0");
                QAction* act = sizeMenu->addAction(
                    QString::fromLatin1(sizes[i].label) + keyHint);
                act->setCheckable(true);
                sizeModeGroup_->addAction(act);
                actSizePresets_ << act;

                registry_.on(aid, [this, pw, ph, act]{
                    currentSizeMode_ = SizeMode::Absolute;
                    currentZoom_     = 0;
                    applyAbsoluteSize(pw, ph);
                    act->setChecked(true);
                });
                connect(act, &QAction::triggered, this, [this, aid]{ registry_.trigger(aid); });
            }
        }

        // M4.3: 全画面（F キー / メニュー / ダブルクリック）
        viewMenu->addSeparator();
        actFullScreen_ = viewMenu->addAction(tr("全画面(&F)ull Screen\tF"));
        actFullScreen_->setCheckable(true);
        connect(actFullScreen_, &QAction::triggered, this,
                [this]{ registry_.trigger(ActionId::FullScreen); });

        // M4.4: スナップショット（P キー / O キー）
        viewMenu->addSeparator();
        actSnapshot_ = viewMenu->addAction(tr("スナップショット保存 (&P)napshot\tP"));
        connect(actSnapshot_, &QAction::triggered, this,
                [this]{ registry_.trigger(ActionId::SnapshotSave); });
        actSnapshotFolder_ = viewMenu->addAction(tr("保存フォルダを開く\tO"));
        connect(actSnapshotFolder_, &QAction::triggered, this,
                [this]{ registry_.trigger(ActionId::SnapshotFolder); });

        // M5.2: 設定導線（設定フォルダを開く / 設定を再読み込み）
        viewMenu->addSeparator();
        QAction* actOpenConfigFolder = viewMenu->addAction(tr("設定フォルダを開く"));
        connect(actOpenConfigFolder, &QAction::triggered, this,
                [this]{ registry_.trigger(ActionId::OpenConfigFolder); });
        QAction* actReloadConfig = viewMenu->addAction(tr("設定を再読み込み"));
        connect(actReloadConfig, &QAction::triggered, this,
                [this]{ registry_.trigger(ActionId::ReloadConfig); });
    }

    // M5.5: ウィンドウジオメトリの起動時復元 ————————————————————————————————————
    // resize は show() 前でも有効（Qt はウィンドウ非表示時に仮想ジオメトリを保持）。
    // move はオフスクリーン復元を防ぐため、復元座標が可視スクリーン上にある場合のみ適用する。
    {
        const auto geo = restoredGeometry(config_.restore, config_.state);
        resize(geo.w, geo.h);
        if (geo.applyPosition) {
            const QPoint pos(geo.x, geo.y);
            if (QGuiApplication::screenAt(pos) != nullptr) {
                move(pos);
            }
        }
    }

    // ウィンドウタイトルの初期値
    setWindowTitle(QString::fromLatin1(common::appName()));
}

MainWindow::~MainWindow() = default;

void MainWindow::openMedia(const QString& path,
                            const QString& name,
                            const QString& contact,
                            bool           commandline)
{
    if (!attached_) {
        // show() がまだ来ていない — 引数を保持して showEvent で再呼び出す
        pending_ = {true, path, name, contact, commandline};
        return;
    }
    session_->start(path, name, contact, commandline);
}

void MainWindow::showEvent(QShowEvent* event) {
    QMainWindow::showEvent(event);
    if (!attached_) {
        attachMpv();
        if (pending_.valid) {
            pending_.valid = false;
            session_->start(pending_.path, pending_.name,
                            pending_.contact, pending_.commandline);
        }
    }
}

void MainWindow::attachMpv() {
    if (attached_) { return; }
    attached_ = true;

    // winId() は show() 後に実体化される。
    // VideoHostWidget の WA_NativeWindow により HWND が確保されている。
    // WId(=HWND__*) を quintptr に reinterpret_cast して MpvBackend に渡す。
    const auto wid = static_cast<quintptr>(videoWidget_->winId());
    if (!mpv_->attach(wid)) {
        qWarning() << "[mpv] attach 失敗";
        statusBar()->showMessage(tr("mpv の初期化に失敗しました"));
        return;
    }

    // M5.3: attach 成功後に復元した音量/ミュートを mpv に初期適用する。
    // attach 前に setProperty を呼んでも mpv が未 init のため no-op になるのでここで行う。
    mpv_->setPropertyDouble(QStringLiteral("volume"), static_cast<double>(currentVolume_));
    applyMute();
}

void MainWindow::onTitleChanged(const QString& title) {
    setWindowTitle(QStringLiteral("%1  —  %2")
                       .arg(QString::fromLatin1(common::appName()), title));
}

void MainWindow::onStatusMessage(const QString& msg) {
    statusBar()->showMessage(msg);
}

void MainWindow::onMpvLogMessage(const QString& prefix,
                                  const QString& level,
                                  const QString& text)
{
    // エラー/警告のみステータスバーとデバッグ出力に表示する
    if (level == QLatin1String("error") || level == QLatin1String("warn")) {
        qDebug() << "[mpv]" << prefix << level << text;
        statusBar()->showMessage(QStringLiteral("[mpv/%1] %2").arg(prefix, text));
    }
}

// M4.1: 現在の currentFit_/currentAspectX_/Y_ を mpv に適用する。
// mpv 未 init 時（mpv_ ガード）は setProperty が no-op になるため安全。
void MainWindow::applyFitMode()
{
    const auto props = fitModeToMpvProps(currentFit_, currentAspectX_, currentAspectY_);
    for (const auto& p : props) {
        mpv_->setProperty(p.name, p.value);
    }
}

// M4.2: ズーム%をネイティブ映像サイズに掛けた目標ピクセルで videoWidget_ を固定する。
// lastVideoW_/H_ が未取得（0）のときは videoTargetForZoom が QSize(0,0) を返すため no-op。
void MainWindow::applyZoom(int percent)
{
    const QSize t = videoTargetForZoom(lastVideoW_, lastVideoH_, percent);
    if (t.isEmpty()) { return; }   // native 未取得 → no-op（M4.0 の既存ガードを活用）
    fixVideoWidgetSize(t);
}

// M4.2: 映像領域をちょうど w×h ピクセルに固定する（バー除外の正確ピクセル = OBS タイル配信用）。
// ウィンドウ全体はタイトル帯など下部バーの高さ分だけ大きくなる（Qt レイアウト自動加算）。
void MainWindow::applyAbsoluteSize(int w, int h)
{
    if (w <= 0 || h <= 0) { return; }
    lastAbsW_ = w; lastAbsH_ = h;    // M4.3: 全画面復帰時の再適用用に保持
    fixVideoWidgetSize(QSize(w, h));
}

// videoWidget_ を target に固定し、ウィンドウを拡大・縮小の両方向に追従させる。
// videoWidget_ は videoResListSplitter_（縦 QSplitter）の子なので、setFixedSize だけでは
// QSplitter が内部のサイズ配分を更新せず、縮小時にスプリッター→ウィンドウの sizeHint が
// 縮まない（拡大は効くが縮小が効かない、という非対称な不具合になる）。
// スプリッターへ明示的に映像ペインの高さを push してキャッシュを無効化してから縮める。
void MainWindow::fixVideoWidgetSize(const QSize& target)
{
    videoWidget_->setFixedSize(target);

    // QSplitter に映像ペインの新しい高さを反映（レス一覧ペインの高さは保持）。
    // 縦スプリッターなので幅は activate()+adjustSize() 側で追従する。
    if (videoResListSplitter_) {
        QList<int> sizes = videoResListSplitter_->sizes();
        if (!sizes.isEmpty()) {
            sizes[0] = target.height();
            videoResListSplitter_->setSizes(sizes);
        }
        videoResListSplitter_->updateGeometry();
    }

    // setFixedSize 直後は QMainWindowLayout のサイズヒントがキャッシュを返すため、
    // 縮小方向への adjustSize() が効かない。activate() で強制再計算してから縮める。
    centralWidget()->layout()->activate();
    adjustSize();
}

// M4.2: setFixedSize を解除して自由リサイズに戻す。
// setMinimumSize で下限のみ復元し、setMaximumSize でブロックを撤去する。
void MainWindow::releaseSizeFixed()
{
    videoWidget_->setMinimumSize(640, 360);
    videoWidget_->setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
    adjustSize();
}

// M4.3: 全画面トグル ———————————————————————————————————————————————————————

// F キー / メニュー / ダブルクリックの共通エントリ。
void MainWindow::toggleFullScreen()
{
    if (isFullScreen()) { leaveFullScreen(); } else { enterFullScreen(); }
}

// 全画面入場: M4.2 サイズ固定を一時解除 → showFullScreen()。
// PCRPlayer 仕様: UI（ステータスバー・書き込み欄等）はフルスクリーン中も維持する。
// currentSizeMode_ は変更しない（状態保持）。
void MainWindow::enterFullScreen()
{
    // M4.2 のサイズ固定を一時解除（固定箱のまま全画面にすると映像が画面を埋めない）。
    // currentSizeMode_ は保持し、leaveFullScreen() で再適用する。
    videoWidget_->setMinimumSize(640, 360);
    videoWidget_->setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);

    showFullScreen();
    if (actFullScreen_) { actFullScreen_->setChecked(true); }
}

// 全画面復帰: showNormal() → サイズモードを再適用。
void MainWindow::leaveFullScreen()
{
    showNormal();
    reapplySizeMode();
    if (actFullScreen_) { actFullScreen_->setChecked(false); }
}

// currentSizeMode_ に応じて適切なサイズ適用関数を呼ぶ。
void MainWindow::reapplySizeMode()
{
    switch (currentSizeMode_) {
        case SizeMode::Zoom:
            applyZoom(currentZoom_);
            break;
        case SizeMode::Absolute:
            applyAbsoluteSize(lastAbsW_, lastAbsH_);
            break;
        case SizeMode::Free:
            releaseSizeFixed();
            break;
    }
}

// M5.1: 中央キーディスパッチャ ————————————————————————————————————————————————
//
// 横断決定4: 全キー入力をここで一元処理する。
//   1. レス入力欄フォーカス中は素通し（BBS 書き込み中にプレイヤーキーを発火させない）
//   2. Tab で入力欄 ⇄ プレイヤーのフォーカスを往復（PCRPlayer IDM_WINDOW_EDIT 踏襲）
//   3. Esc は全画面中のみ解除（既存特例を維持）
//   4. それ以外は KeyChord に正規化してレジストリへ dispatch
void MainWindow::keyPressEvent(QKeyEvent* event)
{
    // 1. レス入力欄フォーカス中は QMainWindow に素通し（resInputBar_ が消費する）
    if (resInputBar_->isVisible() && resInputBar_->inputHasFocus()) {
        QMainWindow::keyPressEvent(event);
        return;
    }

    // 2. Tab → レス入力欄（表示中かつ有効なら）にフォーカスを移す
    if (event->key() == Qt::Key_Tab) {
        if (resInputBar_->isVisible() && resInputBar_->isEnabled()) {
            resInputBar_->setInputFocus();
            event->accept();
        } else {
            QMainWindow::keyPressEvent(event);
        }
        return;
    }

    // 3. Esc: 全画面中のみ解除（レジストリに乗せず特例として維持）
    if (event->key() == Qt::Key_Escape && isFullScreen()) {
        leaveFullScreen();
        event->accept();
        return;
    }

    // 4. レジストリ経由ディスパッチ
    const KeyChord chord = keyChordFromEvent(event);
    if (chord.key != 0 && registry_.dispatch(chord)) {
        event->accept();
        return;
    }

    QMainWindow::keyPressEvent(event);
}

// ダブルクリックで全画面トグル。
// 注意: --wid 埋め込みでは mpv が映像領域に子 HWND を置くため、映像上のダブルクリックは
//       Qt に届かない場合がある（ベストエフォート）。非映像領域（タイトル帯など）では届く。
void MainWindow::mouseDoubleClickEvent(QMouseEvent* event)
{
    toggleFullScreen();
    QMainWindow::mouseDoubleClickEvent(event);
}

// M4.4: スナップショット ——————————————————————————————————————————————————————

// スナップショットの保存先ディレクトリを返す。ディレクトリ自体は作成しない。
// ハードコード既定（Pictures/YAPCRPlayer）。// M5: config化（保存先フォルダ）
QString MainWindow::snapshotDirectory() const
{
    const QString base =
        QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
    return base + QStringLiteral("/YAPCRPlayer");
}

// 現在の映像フレームを OSD/字幕なしの素フレームで保存する。
// video フラグ = OSD/字幕なし・元解像度。無映像時は mpv が書き出しに失敗するが M4 ではガードしない。
// 形式 PNG 固定。// M5: config化（形式・JPEG 品質）
void MainWindow::takeSnapshot()
{
    const QString dir  = snapshotDirectory();
    QDir().mkpath(dir);  // 保存先フォルダが無ければ自動作成
    const QString path = dir + QStringLiteral("/")
                       + snapshotFilename(QDateTime::currentDateTime(), SnapshotFormat::Png);
    // screenshot-to-file <path> <flags>: 拡張子から形式を自動判定（.png→PNG）
    mpv_->command({ QStringLiteral("screenshot-to-file"), path, QStringLiteral("video") });
    onStatusMessage(tr("スナップショットを保存: %1").arg(path));
}

// 保存フォルダをエクスプローラで開く。フォルダが無ければ作成してから開く。
void MainWindow::openSnapshotFolder()
{
    const QString dir = snapshotDirectory();
    QDir().mkpath(dir);
    QDesktopServices::openUrl(QUrl::fromLocalFile(dir));
}

// M5.3: 音量を delta ステップ変化させて mpv に適用する。
// 0-100 にクランプしてステータスバーに通知する。
void MainWindow::changeVolume(int delta)
{
    currentVolume_ = applyVolumeStep(currentVolume_, delta);
    mpv_->setPropertyDouble(QStringLiteral("volume"), static_cast<double>(currentVolume_));
    statusBar()->showMessage(tr("音量: %1%").arg(currentVolume_), 2000);
}

// M5.3: muteState_.effective() を mpv の mute プロパティに反映し、メニューを同期する。
void MainWindow::applyMute()
{
    mpv_->setPropertyFlag(QStringLiteral("mute"), muteState_.effective());
    if (actMute_) { actMute_->setChecked(muteState_.userMute()); }
}

// M5.3: ウィンドウ状態変化（最小化/復帰）を検出して連動ミュートを制御する。
// minimize_mute 機能は changeEvent で一元処理するため、最小化の経路（タスクバー/
// Minimize アクション/Alt+F9 等）に依らず一貫した動作になる。
void MainWindow::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::WindowStateChange) {
        auto* ev = static_cast<QWindowStateChangeEvent*>(event);
        const bool wasMin = (ev->oldState() & Qt::WindowMinimized) != 0;
        const bool nowMin = (windowState()  & Qt::WindowMinimized) != 0;
        if (!wasMin && nowMin) {
            // 最小化: minimize_mute が ON かつ手動ミュートなし → 自動ミュート
            muteState_.onMinimize(config_.playback.minimize_mute);
            applyMute();
        } else if (wasMin && !nowMin) {
            // 復帰: 自動ミュートのみ解除（ユーザーミュートは保持）
            muteState_.onRestore();
            applyMute();
        }
    }
    QMainWindow::changeEvent(event);
}

// M5.5: 終了時に [state] を現在値に更新して config.toml を書き出す。
// QuitStop アクション起動（quitStopRequested_=true）または [general].quit_stop=true のとき
// 終了前にチャンネル切断（PCRPlayer MainDlg.cpp:711 の network.stop 分岐に相当）。
void MainWindow::closeEvent(QCloseEvent* event)
{
    // チャンネル切断
    if (quitStopRequested_ || config_.general.quit_stop) {
        session_->manualStop();
    }

    // [state] を現在値に更新する。
    // normalGeometry() は最小化・最大化・全画面時でも通常ウィンドウの位置/サイズを返す。
    const QRect g = normalGeometry();
    config_.state.window_x = g.x();
    config_.state.window_y = g.y();
    config_.state.window_w = g.width();
    config_.state.window_h = g.height();
    config_.state.volume   = currentVolume_;
    config_.state.mute     = muteState_.userMute();
    // sage はロード値をそのまま保持（live トグル未実装）。データ欠落させずに書き戻す。

    config::save(config_, configPath_);
    QMainWindow::closeEvent(event);
}

// 映像子ウィンドウ（mpv --wid）のクリックを検出して MainWindow にフォーカスを戻す。
// mpv は VideoHostWidget 内に子 HWND を置くため Qt のマウスイベントが届かない。
// WM_PARENTNOTIFY は子 HWND でのマウスボタン押下を祖先ウィンドウに伝播させる。
bool MainWindow::nativeEvent(const QByteArray& eventType, void* message, qintptr* result)
{
#ifdef Q_OS_WIN
    if (eventType == "windows_generic_MSG") {
        const MSG* msg = static_cast<const MSG*>(message);
        if (msg->message == WM_PARENTNOTIFY) {
            const UINT childMsg = LOWORD(msg->wParam);
            if (childMsg == WM_LBUTTONDOWN || childMsg == WM_RBUTTONDOWN) {
                setFocus(Qt::MouseFocusReason);
            }
        }
    }
#endif
    return QMainWindow::nativeEvent(eventType, message, result);
}

}  // namespace yapcr::app
