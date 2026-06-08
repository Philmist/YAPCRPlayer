#include <QApplication>

#include "main_window.h"

int main(int argc, char** argv) {
    QApplication app(argc, argv);

    yapcr::ui::MainWindow window;
    window.resize(960, 540);
    window.show();

    return app.exec();
}
