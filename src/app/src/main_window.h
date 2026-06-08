#pragma once

#include <QMainWindow>

namespace yapcr::ui {

// M0 のスケルトンウィンドウ。
// ウィンドウタイトルに Qt と libmpv のバージョンを出し、ツールチェーン疎通を示す。
// 中央は将来の VideoHostWidget(--wid 埋め込み) の占位。
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
};

}  // namespace yapcr::ui
