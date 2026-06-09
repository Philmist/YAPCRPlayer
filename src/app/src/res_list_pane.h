#pragma once

#include "bbs/models.h"
#include "res_popup.h"

#include <functional>
#include <QList>
#include <QMouseEvent>
#include <QPlainTextEdit>

namespace yapcr::app {

// 簡易レス一覧ペイン（M3.6/M3.7）
//
// QPlainTextEdit ベースの読み取り専用スクロール可能テキスト一覧。
// 整形: 1レス = "番号 名前 [ID] 日時\n本文" のプレーンテキスト行。
// 差分追記: 既出件数を保持し新規分のみ appendPlainText で追記する。
// M3.7: ヘッダ行ホバーで byRef ポップアップ、本文 >>N ホバーで byRange ポップアップを表示。
class ResListPane : public QPlainTextEdit {
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

    // M3.7: hover ポップアップ用クエリプロバイダを注入する（MainWindow から呼ぶ）。
    using ResQueryFn   = std::function<QList<yapcr::bbs::ResInfo>(int)>;
    using RangeQueryFn = std::function<QList<yapcr::bbs::ResInfo>(yapcr::bbs::Range)>;
    void setByRefProvider(ResQueryFn fn);
    void setByRangeProvider(RangeQueryFn fn);

protected:
    void mouseMoveEvent(QMouseEvent* event) override;
    void leaveEvent(QEvent* event) override;

private:
    // カーソル位置がレスのヘッダ行（先頭ブロック）にあれば正のレス番号を返す。
    int resNumberAt(QPoint vpos) const;

    // カーソル位置が本文行内のアンカー >>N にあればその N を返す。なければ -1。
    int anchorTargetAt(QPoint vpos) const;

    int     displayedCount_{0};
    QString threadTitle_;

    ResPopup*    popup_{nullptr};
    int          lastHoveredRes_{-1};
    ResQueryFn   byRefProvider_;
    RangeQueryFn byRangeProvider_;
};

}  // namespace yapcr::app
