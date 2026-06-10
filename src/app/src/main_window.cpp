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
#include "window_geometry.h"  // M4.2: videoTargetForZoom / zoomPresets / sizePresets

#include <QAction>
#include <QActionGroup>
#include <QCursor>
#include <QDebug>
#include <QDockWidget>
#include <QKeyEvent>
#include <QMenu>
#include <QMenuBar>
#include <QMouseEvent>
#include <QShowEvent>
#include <QStatusBar>
#include <QString>
#include <QVBoxLayout>
#include <QtGlobal>

namespace yapcr::app {

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    // M3.7: centralWidget を QWidget コンテナ化し、映像ウィジェットと掲示板タイトル帯を縦積みする。
    // 注意: videoWidget_->winId() は attachMpv()（showEvent 後）で mpv に渡す。
    //       コンテナ化は attach 前に確定させ、attach 後に reparent しないこと。
    {
        QWidget* central = new QWidget(this);
        QVBoxLayout* vl  = new QVBoxLayout(central);
        vl->setContentsMargins(0, 0, 0, 0);
        vl->setSpacing(0);

        videoWidget_ = new VideoHostWidget(central);
        videoWidget_->setMinimumSize(640, 360);
        vl->addWidget(videoWidget_, 1);

        boardTitleBar_ = new BoardTitleBar(central);
        vl->addWidget(boardTitleBar_, 0);

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

    // M3.6: BBS レス一覧ペイン（右ドック、初期は非表示）
    resListPane_ = new ResListPane(this);
    resDock_ = new QDockWidget(tr("BBS レス一覧"), this);
    resDock_->setObjectName(QStringLiteral("resDock"));
    resDock_->setWidget(resListPane_);
    addDockWidget(Qt::RightDockWidgetArea, resDock_);
    resDock_->hide();

    // M3.6: BBS レス入力バー（下部ドック、初期は非表示）
    resInputBar_ = new ResInputBar(this);
    inputDock_ = new QDockWidget(tr("BBS 書き込み"), this);
    inputDock_->setObjectName(QStringLiteral("inputDock"));
    inputDock_->setWidget(resInputBar_);
    addDockWidget(Qt::BottomDockWidgetArea, inputDock_);
    inputDock_->hide();

    // M3.6: BBS シグナル配線
    connect(session_, &SessionController::bbsResAppended,
            this, [this](const QList<yapcr::bbs::ResInfo>& resList) {
                resListPane_->appendResList(resList);
                // 書き込みバーは初回受信時に自動表示する。
                // レス一覧ドックはユーザーが明示的に開く（hover ポップアップで代替できる）。
                if (!resList.isEmpty()) {
                    inputDock_->show();
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

    // M2: 最小メニューバー（再接続/切断/再読込）
    {
        QMenu* menu = menuBar()->addMenu(tr("操作(&O)"));

        actBump_   = menu->addAction(tr("再接続 (&B)ump"));
        actStop_   = menu->addAction(tr("切断 (&S)top"));
        menu->addSeparator();
        actReload_ = menu->addAction(tr("再読込 (&R)eload"));

        connect(actBump_,   &QAction::triggered, session_, &SessionController::manualBump);
        connect(actStop_,   &QAction::triggered, session_, &SessionController::manualStop);
        connect(actReload_, &QAction::triggered, session_, &SessionController::manualReload);

        // M3.6: BBS 取得/更新・ドック表示切替
        menu->addSeparator();
        actBbsRefresh_ = menu->addAction(tr("BBS 取得/更新 (&G)"));
        connect(actBbsRefresh_, &QAction::triggered, this, [this] {
            resListPane_->clearRes();
            session_->bbsRefresh();
        });
        menu->addSeparator();
        QAction* toggleResDock   = resDock_->toggleViewAction();
        QAction* toggleInputDock = inputDock_->toggleViewAction();
        toggleResDock->setText(tr("レス一覧 (&L) 表示切替"));
        toggleInputDock->setText(tr("書き込み欄 (&W) 表示切替"));
        menu->addAction(toggleResDock);
        menu->addAction(toggleInputDock);
    }

    // M4.1: 「表示」メニュー（フィットモード・アスペクト比）
    {
        QMenu* viewMenu = menuBar()->addMenu(tr("表示(&V)"));

        // フィットモードとアスペクト比は同一の排他グループとする。
        // モード選択は実質一軸なので、1 つの QActionGroup を 2 サブメニューに配置する。
        displayModeGroup_ = new QActionGroup(this);
        displayModeGroup_->setExclusive(true);

        // ---- フィットサブメニュー ----
        QMenu* fitMenu = viewMenu->addMenu(tr("フィット(&F)"));

        auto addFit = [&](const QString& label, FitMode mode, int ax = 0, int ay = 0) {
            QAction* act = fitMenu->addAction(label);
            act->setCheckable(true);
            displayModeGroup_->addAction(act);
            connect(act, &QAction::triggered, this, [this, mode, ax, ay] {
                currentFit_     = mode;
                currentAspectX_ = ax;
                currentAspectY_ = ay;
                applyFitMode();
            });
            return act;
        };

        QAction* actInscribe = addFit(tr("内接 (&I)nscribe（既定）"), FitMode::Inscribe);
        actInscribe->setChecked(true);  // 初期選択
        addFit(tr("引き伸ばし (&S)tretch"),    FitMode::Stretch);
        addFit(tr("充填 (&F)ill"),             FitMode::Fill);
        addFit(tr("等倍 (&U)nscaled"),         FitMode::Unscaled);

        // ---- アスペクト比サブメニュー ----
        QMenu* aspectMenu = viewMenu->addMenu(tr("アスペクト比(&A)"));

        auto addAspect = [&](const QString& label, FitMode mode, int ax, int ay) {
            QAction* act = aspectMenu->addAction(label);
            act->setCheckable(true);
            displayModeGroup_->addAction(act);
            connect(act, &QAction::triggered, this, [this, mode, ax, ay] {
                currentFit_     = mode;
                currentAspectX_ = ax;
                currentAspectY_ = ay;
                applyFitMode();
            });
            return act;
        };

        // 「なし」= 内接相当（フィットサブメニューの内接と同じ効果）
        addAspect(tr("なし（内接）"), FitMode::Inscribe, 0, 0);

        // プリセット（ハードコード既定。M5: config化）
        for (const auto& preset : aspectPresets()) {
            addAspect(QString::fromLatin1(preset.label),
                      FitMode::AspectOverride, preset.x, preset.y);
        }

        viewMenu->addSeparator();

        // ---- M4.2: サイズ固定（ズーム% / 絶対サイズ）----
        // フィット/アスペクト（displayModeGroup_）とは別軸の排他グループ。
        sizeModeGroup_ = new QActionGroup(this);
        sizeModeGroup_->setExclusive(true);

        // 「自由リサイズ（固定解除）」— 初期選択
        {
            QAction* actFree = viewMenu->addAction(tr("サイズ固定を解除（自由リサイズ）"));
            actFree->setCheckable(true);
            actFree->setChecked(true);
            sizeModeGroup_->addAction(actFree);
            connect(actFree, &QAction::triggered, this, [this] {
                currentSizeMode_ = SizeMode::Free;
                currentZoom_     = 0;
                releaseSizeFixed();
            });
        }

        // 「ズーム%」サブメニュー（ラジオ）: プリセットをループして生成
        {
            QMenu* zoomMenu = viewMenu->addMenu(tr("ズーム(&Z)"));
            for (int percent : zoomPresets()) {
                QAction* act = zoomMenu->addAction(QStringLiteral("%1%").arg(percent));
                act->setCheckable(true);
                sizeModeGroup_->addAction(act);
                connect(act, &QAction::triggered, this, [this, percent] {
                    currentSizeMode_ = SizeMode::Zoom;
                    currentZoom_     = percent;
                    applyZoom(percent);
                });
            }
        }

        // 「絶対サイズ」サブメニュー（ラジオ）: プリセットをループして生成
        {
            QMenu* sizeMenu = viewMenu->addMenu(tr("絶対サイズ(&S)"));
            for (const auto& preset : sizePresets()) {
                QAction* act = sizeMenu->addAction(QString::fromLatin1(preset.label));
                act->setCheckable(true);
                sizeModeGroup_->addAction(act);
                const int pw = preset.w;
                const int ph = preset.h;
                connect(act, &QAction::triggered, this, [this, pw, ph] {
                    currentSizeMode_ = SizeMode::Absolute;
                    currentZoom_     = 0;
                    applyAbsoluteSize(pw, ph);
                });
            }
        }

        // M4.3: 全画面（M5: config化 — 全画面時のバー表示有無）
        viewMenu->addSeparator();
        actFullScreen_ = viewMenu->addAction(tr("全画面(&F)ull Screen\tF"));
        actFullScreen_->setCheckable(true);
        connect(actFullScreen_, &QAction::triggered, this, [this] { toggleFullScreen(); });
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
    }
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
    videoWidget_->setFixedSize(t);
    // setFixedSize 直後は QMainWindowLayout のサイズヒントがキャッシュを返すため、
    // 縮小方向への adjustSize() が効かない。activate() で強制再計算してから縮める。
    centralWidget()->layout()->activate();
    adjustSize();
}

// M4.2: 映像領域をちょうど w×h ピクセルに固定する（バー除外の正確ピクセル = OBS タイル配信用）。
// ウィンドウ全体はタイトル帯など下部バーの高さ分だけ大きくなる（Qt レイアウト自動加算）。
void MainWindow::applyAbsoluteSize(int w, int h)
{
    if (w <= 0 || h <= 0) { return; }
    lastAbsW_ = w; lastAbsH_ = h;    // M4.3: 全画面復帰時の再適用用に保持
    videoWidget_->setFixedSize(w, h);
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

// 全画面入場: バーを退避 → M4.2 サイズ固定を一時解除 → showFullScreen()。
// currentSizeMode_ は変更しない（状態保持）。
void MainWindow::enterFullScreen()
{
    // 入場前のバー表示状態を退避（復帰時に正確に戻すため）
    savedBars_ = {
        menuBar()->isVisible(),
        statusBar()->isVisible(),
        boardTitleBar_->isVisible(),
        resDock_->isVisible(),
        inputDock_->isVisible()
    };

    // クリーンな映像（Q5=A）: 全バー/ドックを隠す。// M5: config化（全画面時のバー表示有無）
    menuBar()->hide();
    statusBar()->hide();
    boardTitleBar_->hide();
    resDock_->hide();
    inputDock_->hide();

    // M4.2 のサイズ固定を一時解除（固定箱のまま全画面にすると映像が画面を埋めない）。
    // currentSizeMode_ は保持し、leaveFullScreen() で再適用する。
    videoWidget_->setMinimumSize(640, 360);
    videoWidget_->setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);

    showFullScreen();
    if (actFullScreen_) { actFullScreen_->setChecked(true); }
}

// 全画面復帰: showNormal() → バーを退避状態に戻す → サイズモードを再適用。
void MainWindow::leaveFullScreen()
{
    showNormal();

    // バーを入場前の状態に復帰する
    menuBar()->setVisible(savedBars_.menuBar);
    statusBar()->setVisible(savedBars_.statusBar);
    boardTitleBar_->setVisible(savedBars_.titleBar);
    resDock_->setVisible(savedBars_.resDock);
    inputDock_->setVisible(savedBars_.inputDock);

    // 選択中のサイズモードを再適用（ズーム/絶対サイズの固定を元に戻す）
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

// F キー: 全画面トグル。Esc キー: 全画面中のみ解除。
// resInputBar_ の入力欄にフォーカスがある間はそちらが消費するため、書き込み中はトグルされない（望ましい）。
void MainWindow::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_F) {
        toggleFullScreen();
        return;
    }
    if (event->key() == Qt::Key_Escape && isFullScreen()) {
        leaveFullScreen();
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

}  // namespace yapcr::app
