#pragma once

#include "bbs/models.h"

#include <QList>
#include <QTextDocument>
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

    // [Recent モード] 表示中に新着レス等でデータが更新されたとき、内容を差し替える。
    // 最新に張り付いて見ていた場合は新しい最新へ追従し、過去を遡行中なら位置を保つ。
    // 配置は保存したアンカーから再計算する（伸長してもタイトル帯の直上に収める）。
    // Recent モードで表示中でなければ何もしない。
    void refreshRecent(const QList<yapcr::bbs::ResInfo>& all);

    // Recent モードで現在表示中か（呼び出し側がデータ更新の要否を判定するため）。
    bool isRecentVisible() const { return recentMode_ && isVisible(); }

    // Recent モードで現在描画中の可視レス（トリム後の窓）。非表示・非 Recent 時は空。
    // 右クリックメニューの URL 抽出など、「フロートに今見えているレス」を要する呼び出し側用。
    QList<yapcr::bbs::ResInfo> visibleReses() const;

    // [Recent モード] 表示中の窓を delta 方向にスライドする（タイトル帯からの転送用）。
    // delta > 0（上回し）で過去へ、delta < 0（下回し）で最新へ。
    // Recent モードで表示中でなければ何もしない。
    void scrollRecent(int delta);

    // [Recent モード] 遡行状態を最新基点へ戻す（スレッド切替時に呼ぶ）。
    // 別スレッドのレス番号にアンカーが誤マッチするのを防ぐ。
    void resetScroll();

    void hidePopup();

protected:
    void paintEvent(QPaintEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void leaveEvent(QEvent* event) override;

private:
    void  rebuildText();
    // Recent モードの表示 HTML を組み立てる（begin..end のレス＋範囲フッタ）。
    QString buildRecentHtml(int begin, int end) const;
    // 現在 doc_ に設定済みの HTML を確定幅で割り付けた内容サイズ（パディング含まず）。
    QSizeF layoutContent();
    // doc_ の内容から表示サイズを算出する（kMaxWidth/kMaxHeight でクランプ）。
    QSize computeSize();
    // 保存した recentAnchor_ を基点に、タイトル帯の直上へ配置する（現在のサイズ基準）。
    void  placeRecent();
    // followLatest_/anchorNumber_ から recentAll_ 基準で windowEnd_ を決める。
    void  applyWindowEnd();

    // ---- Single モード ----
    QList<yapcr::bbs::ResInfo> resList_;
    int     wheelIndex_{0};

    // ---- Recent モード ----
    QList<yapcr::bbs::ResInfo> recentAll_;
    int     windowEnd_{0};  // 現在表示する末尾レスのインデックス（1始まり → サイズ基準）
    int     windowBegin_{0};  // rebuildText で確定した可視窓の先頭（トリム後）
    bool    followLatest_{true};  // 最新レスに張り付いているか（初期＝最新基点）
    QString anchorNumber_;  // 非追従時に末尾可視だったレス番号（hide/show を跨ぐ復元の基点）
    QPoint  recentAnchor_;  // 初回 showRecent 時のタイトル帯アンカー（再配置の基点）

    // ---- 共通 ----
    bool          recentMode_{false};
    QTextDocument doc_;  // 表示用リッチテキスト（一覧ペインと同配色）

    static constexpr int   kPadding   = 8;
    static constexpr int   kMaxWidth  = 600;
    static constexpr int   kMaxHeight = 640;  // この高さで収まらない分は最古レスから落とす
    static constexpr qreal kAlpha     = 0.92;  // ライト半透明背景の不透明度
    static constexpr int   kWindow    = 10;  // Recent モードで一度に表示するレス数
};

}  // namespace yapcr::app
