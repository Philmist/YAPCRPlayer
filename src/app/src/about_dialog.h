#pragma once

#include <QDialog>

namespace yapcr::app {

// M6: バージョン情報ダイアログ。
//
// 表示内容:
//   - アプリ名 / バージョン
//   - ライセンス（GPL-3.0-or-later）
//   - サードパーティコンポーネントのクレジット（libmpv / Qt / FFmpeg / toml++）
//   - 対応ソース入手先 URL
//
// Version アクション（L キーの Log は別途）から呼び出す。
class AboutDialog : public QDialog {
    Q_OBJECT

public:
    explicit AboutDialog(QWidget* parent = nullptr);
};

}  // namespace yapcr::app
