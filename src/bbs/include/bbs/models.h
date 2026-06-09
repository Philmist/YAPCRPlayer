#pragma once

#include <QString>
#include <QList>
#include <QPair>
#include <cstdlib>  // std::abs

namespace yapcr::bbs {

// アンカー範囲（移植元: BBSRegex.h:8-16 の Range）
// first と last を含む閉区間。単一レス参照は first == last。
struct Range {
    int first{0};
    int last{0};

    // 範囲の幅（移植元 range() は型名と紛れるため span() に改名）
    int  span()        const { return std::abs(first - last); }
    // v が範囲内かを返す
    bool within(int v) const { return v >= first && v <= last; }
};


// 文字コード（移植元: Mlang::CODE）
enum class Charset {
    ShiftJis,  // jpnkn / 2ch 系（Shift_JIS）
    EucJp,     // したらば（EUC-JP）
};

// 板種別
enum class BoardType {
    Jpnkn,     // bbs.jpnkn.com 互換（2ch 系）
    Shitaraba, // jbbs.shitaraba.net（したらば）
};

// 板情報（移植元: BBSInfo::Board, BaseBBS.h:37-54）
// std::wstring → QString、Mlang::CODE → Charset
struct Board {
    Charset code{Charset::ShiftJis};  // 文字コード

    QString scheme;   // スキーム（http / https）
    QString host;     // ホスト（例: bbs.jpnkn.com）
    QString base;     // ベース URL
                      //   jpnkn:     host＋パス接頭辞込み（通常はホストと同一）
                      //   したらば:  host のみ
    QString board;    // 板名（例: livegame）
    QString number;   // 板番号（したらば のみ使用。jpnkn は常に空）

    QString title;    // 板タイトル（setting.txt の BBS_TITLE）
    QString noname;   // デフォルト名無し（setting.txt の BBS_NONAME_NAME）
    QString url;      // 板 URL（board() ビルダーで生成）
};

// スレッド情報（移植元: BBSInfo::Thread, BaseBBS.h:56-71）
struct Thread {
    QString key;        // スレッドキー（UNIX 時刻、例: "1770907315"）
    QString title;      // スレッドタイトル（subject.txt / dat の>>1）
    QString url;        // スレッド URL（thread() ビルダーで生成）
    int     count{0};   // レス数
    int     stop{1000}; // 終了レス数（setting.txt の BBS_THREAD_STOP）
};

// スレッド一覧エントリ（移植元: ThreadInfo, BaseBBS.h:73-85）
// subject.txt の各行から生成。M3.2 で使用。
struct ThreadInfo {
    QString number;        // 一覧上の番号
    QString key;           // スレッドキー（UNIX 時刻）
    QString title;         // タイトル
    int     count{0};      // レス数
    double  speed{0.0};    // 速度（レス/日）
};

// レス情報（移植元: ResInfo, BaseBBS.h:87-114）
// dat の各行から生成。M3.3 で使用。
struct ResInfo {
    bool    del{false};     // 削除済み
    bool    latest{true};   // 最新レス

    QString number;         // レス番号
    QString name;           // 名前
    QString mail;           // メール
    QString datetime;       // 日時文字列
    QString message;        // レス本文
    QString title;          // スレッドタイトル（>>1 のみ、以降は空）
    QString id;             // ID

    int count{0};           // ID の出現数
    int total{0};           // ID の合計レス数
    int identifier{0};      // ID 識別番号

    QList<QString>              link;   // 抽出 URL 一覧
    QList<int>                  ref;    // 被参照レス番号一覧
    QList<QPair<QString, bool>> split;  // <b> 区切り分割断片（M3.3 で充填）
};

}  // namespace yapcr::bbs
