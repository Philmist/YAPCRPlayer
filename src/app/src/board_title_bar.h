#pragma once

#include <QWidget>

class QEnterEvent;

namespace yapcr::app {

// 映像下部に配置する掲示板タイトル帯（M3.7 本命実装）
//
// 表示形式: "[ <title> ]( <count> )"（PCRPlayer 参照画像準拠）
// カーソルが帯に入ると hovered(globalPos) を emit し、
// 帯を離れると left() を emit する。
// ResPopup の showRecent() と組み合わせて直近レスをオーバーレイ表示するトリガとなる。
class BoardTitleBar : public QWidget {
    Q_OBJECT

public:
    explicit BoardTitleBar(QWidget* parent = nullptr);

    // タイトルと件数を更新する（BbsSession の datLoaded 後に呼ぶ）
    void setInfo(const QString& title, int count);

signals:
    // カーソルが帯に入った・帯上で動いた（グローバル座標）
    void hovered(QPoint globalPos);
    // カーソルが帯から出た
    void left();

protected:
    void enterEvent(QEnterEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void paintEvent(QPaintEvent* event) override;

private:
    QString text_;

    static constexpr int kHeight = 24;
};

}  // namespace yapcr::app
