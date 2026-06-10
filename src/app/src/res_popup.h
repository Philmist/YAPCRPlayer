#pragma once

#include "bbs/models.h"

#include <QList>
#include <QWidget>

namespace yapcr::app {

// M3.7: hover レス・ポップアップ
//
// トップレベル半透明フレームレスウィジェット。フォーカスを奪わない。
//
// 2 つの表示モードを持つ:
//   [Single モード] showAt(): ResListPane のレスヘッダ行や >>N アンカー hover 時。
//                  単一/少数レスを表示し wheelEvent で切り替える。
//   [Recent モード] showRecent(): 掲示板タイトル帯 hover 時（M3.7 本命）。
//                  直近 kWindow 件をスタック表示し、wheelEvent で過去へ遡行できる。
class ResPopup : public QWidget {
    Q_OBJECT

public:
    explicit ResPopup(QWidget* parent = nullptr);

    // [Single モード] resList が空なら hide して返す。非空なら先頭から表示し globalPos 近傍に配置する。
    void showAt(const QList<yapcr::bbs::ResInfo>& resList, QPoint globalPos);

    // [Recent モード] all の末尾 kWindow 件を anchorGlobal の直上に半透明スタック表示する。
    // wheelEvent で窓をスライドして過去レスへ遡行できる。
    // all が空なら hide して返す。
    void showRecent(const QList<yapcr::bbs::ResInfo>& all, QPoint anchorGlobal);

    // [Recent モード] 表示中の窓を delta 方向にスライドする（タイトル帯からの転送用）。
    // delta > 0（上回し）で過去へ、delta < 0（下回し）で最新へ。
    // Recent モードで表示中でなければ何もしない。
    void scrollRecent(int delta);

    void hidePopup();

protected:
    void paintEvent(QPaintEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void leaveEvent(QEvent* event) override;

private:
    void rebuildText();

    // ---- Single モード ----
    QList<yapcr::bbs::ResInfo> resList_;
    int     wheelIndex_{0};

    // ---- Recent モード ----
    QList<yapcr::bbs::ResInfo> recentAll_;
    int     windowEnd_{0};  // 現在表示する末尾レスのインデックス（1始まり → サイズ基準）

    // ---- 共通 ----
    bool    recentMode_{false};
    QString displayText_;

    static constexpr int   kPadding   = 8;
    static constexpr int   kMaxWidth  = 600;
    static constexpr int   kMaxHeight = 400;
    static constexpr qreal kAlpha     = 0.88;
    static constexpr int   kWindow    = 10;  // Recent モードで一度に表示するレス数
};

}  // namespace yapcr::app
