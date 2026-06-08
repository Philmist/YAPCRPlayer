#include "main_window.h"

#include <QLabel>
#include <QString>
#include <QtGlobal>

#include "common/version.h"
#include "player/mpv_backend.h"

namespace yapcr::ui {

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    const unsigned long api = player::mpvClientApiVersion();
    const unsigned major = static_cast<unsigned>(api >> 16);
    const unsigned minor = static_cast<unsigned>(api & 0xffffu);

    setWindowTitle(QStringLiteral("%1 %2  —  Qt %3 / mpv api %4.%5")
                       .arg(QString::fromLatin1(common::appName()))
                       .arg(QString::fromLatin1(common::appVersion()))
                       .arg(QString::fromLatin1(QT_VERSION_STR))
                       .arg(major)
                       .arg(minor));

    auto* placeholder = new QLabel(
        QStringLiteral("M0 skeleton — video host placeholder"), this);
    placeholder->setAlignment(Qt::AlignCenter);
    placeholder->setMinimumSize(640, 360);
    setCentralWidget(placeholder);
}

}  // namespace yapcr::ui
