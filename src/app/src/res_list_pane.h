#pragma once

#include "bbs/models.h"
#include "res_popup.h"

#include <functional>
#include <QList>
#include <QMouseEvent>
#include <QTextBrowser>

namespace yapcr::app {

// 簡易レス一覧ペイン（M3.6/M3.7 → M6.1 リッチテキスト化）
//
// QTextBrowser ベースの読み取り専用スクロール可能リッチテキスト一覧。
// 整形: 1レス = reshtml::resToHtml() の HTML 断片（番号/名前/メール/日時を色分け、
//        本文アンカーを着色）。レス間は <hr> で区切る。
// 差分追記: 既出件数を保持し新規分のみ insertHtml で追記する。
// ホバー: anchorAt() で href を判定し、"ref:N" で byRef ポップアップ、
//         "range:a-b" で byRange ポップアップを表示する。
class ResListPane : public QTextBrowser {
    Q_OBJECT

public:
    explicit ResListPane(QWidget* parent = nullptr);

    // スレッドタイトルを設定する（将来の表示改善用）。
    void setThreadTitle(const QString& title);

    // ResInfo 列を差分追記する。全置換ではなく既出件数以降を追記する。
    // ユーザーが末尾付近にいる場合はスクロールを末尾に追従させる。
    void appendResList(const QList<yapcr::bbs::ResInfo>& resList);

    // 表示をリセットし既出件数を 0 に戻す。BBS 取得/更新前に呼ぶ。
    void clearRes();

    // M6: レス一覧を 1 ページ分下方向 / 上方向にスクロールする（ThreadScrollNext/Prev 用）。
    void scrollNext();
    void scrollPrev();

    // M3.7: hover ポップアップ用クエリプロバイダを注入する（MainWindow から呼ぶ）。
    using ResQueryFn   = std::function<QList<yapcr::bbs::ResInfo>(int)>;
    using RangeQueryFn = std::function<QList<yapcr::bbs::ResInfo>(yapcr::bbs::Range)>;
    void setByRefProvider(ResQueryFn fn);
    void setByRangeProvider(RangeQueryFn fn);

protected:
    void mouseMoveEvent(QMouseEvent* event) override;
    void leaveEvent(QEvent* event) override;

private:
    int     displayedCount_{0};
    QString threadTitle_;

    ResPopup*    popup_{nullptr};
    QString      lastHoveredHref_;  // 直近にホバー中の href（同一連打の再表示抑止）
    ResQueryFn   byRefProvider_;
    RangeQueryFn byRangeProvider_;
};

}  // namespace yapcr::app
