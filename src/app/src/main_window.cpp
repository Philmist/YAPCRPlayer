#include "main_window.h"

#include "video_host_widget.h"
#include "session_controller.h"
#include "player/mpv_backend.h"
#include "common/version.h"

#include <QAction>
#include <QDebug>
#include <QMenu>
#include <QMenuBar>
#include <QShowEvent>
#include <QStatusBar>
#include <QString>
#include <QtGlobal>

namespace yapcr::app {

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    // 映像ウィジェットをセンターに配置
    videoWidget_ = new VideoHostWidget(this);
    videoWidget_->setMinimumSize(640, 360);
    setCentralWidget(videoWidget_);

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
