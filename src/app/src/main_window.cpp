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

#include <QAction>
#include <QCursor>
#include <QDebug>
#include <QDockWidget>
#include <QMenu>
#include <QMenuBar>
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
                // BBS データが届いたらドックを自動表示（初回のみ）
                if (!resList.isEmpty()) {
                    resDock_->show();
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
    // タイトル帯からカーソルが外れたとき、ポップアップ自身の上にいる場合は閉じない
    connect(boardTitleBar_, &BoardTitleBar::left,
            this, [this] {
                const QRect popupRect = recentPopup_->geometry();
                if (!popupRect.contains(QCursor::pos())) {
                    recentPopup_->hidePopup();
                }
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

}  // namespace yapcr::app
