#include <QApplication>
#include <QStringList>

#include <clocale>

#include "main_window.h"

int main(int argc, char** argv) {
    QApplication app(argc, argv);

    // libmpv はオプション値の小数点記号として "." を期待する。
    // QApplication がシステムロケールを設定した後に LC_NUMERIC を "C" に戻す。
    // ref: mpv/client.h — "The LC_NUMERIC locale category must be set to 'C'."
    std::setlocale(LC_NUMERIC, "C");

    yapcr::app::MainWindow window;
    window.resize(960, 540);
    window.show();

    // コマンドライン引数を解析して再生を開始する。
    // PCRPlayer の引数仕様: <path> <name> <contact>
    //   args[1] = path    (再生 URL またはファイルパス)
    //   args[2] = name    (チャンネル名)
    //   args[3] = contact (掲示板 URL)
    // 3 引数指定時のみ commandline 扱いで BBS 自動接続（M3 で消費）。
    // PCRPlayer MainDlg.cpp:304-318 の CommandLine::size()>1/2/3 分岐に忠実。
    const QStringList args = QCoreApplication::arguments();
    if (args.size() >= 2) {
        const QString path       = args.value(1);
        const QString name       = args.value(2);
        const QString contact    = args.value(3);
        const bool    commandline = (args.size() > 3);
        window.openMedia(path, name, contact, commandline);
    }

    return app.exec();
}
