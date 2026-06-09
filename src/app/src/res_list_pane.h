#pragma once

#include "bbs/models.h"

#include <QList>
#include <QPlainTextEdit>

namespace yapcr::app {

// 簡易レス一覧ペイン（M3.6）
//
// QPlainTextEdit ベースの読み取り専用スクロール可能テキスト一覧。
// 整形: 1レス = "番号 名前 [ID] 日時\n本文" のプレーンテキスト行。
// 差分追記: 既出件数を保持し新規分のみ appendPlainText で追記する。
// アンカー/リンク装飾・hover は M3.7 で実装。
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

private:
    int     displayedCount_{0};  // 既にペインに表示済みのレス数
    QString threadTitle_;
};

}  // namespace yapcr::app
