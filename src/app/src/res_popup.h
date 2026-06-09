#pragma once

#include "bbs/models.h"

#include <QList>
#include <QWidget>

namespace yapcr::app {

// M3.7: hover レス・ポップアップ
//
// トップレベル半透明フレームレスウィジェット。フォーカスを奪わない。
// ResListPane のレスヘッダ行や >>N アンカー hover 時に関連レスを表示する。
// wheelEvent で resList_ を遡行できる。
class ResPopup : public QWidget {
    Q_OBJECT

public:
    explicit ResPopup(QWidget* parent = nullptr);

    // resList が空なら hide して返す。非空なら先頭から表示し globalPos 近傍に配置する。
    void showAt(const QList<yapcr::bbs::ResInfo>& resList, QPoint globalPos);

    void hidePopup();

protected:
    void paintEvent(QPaintEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

private:
    void rebuildText();

    QList<yapcr::bbs::ResInfo> resList_;
    int     wheelIndex_{0};
    QString displayText_;

    static constexpr int   kPadding   = 8;
    static constexpr int   kMaxWidth  = 400;
    static constexpr int   kMaxHeight = 300;
    static constexpr qreal kAlpha     = 0.88;
};

}  // namespace yapcr::app
