#pragma once

#include "bbs/models.h"

#include <QByteArray>
#include <QList>
#include <QNetworkCookie>
#include <QString>

namespace yapcr::bbs {

// ---- urlencode ----

// 文字列 s を Charset でバイト化してから URL エンコードする。
// 安全集合: [0-9 a-z A-Z * - . _]（移植元 Util.cpp urlencode と一致）。
//   それ以外は %XX、空白は %20（+ エンコード不使用）。
// encodeTo()（charset.h）でバイト化し、QByteArray::toPercentEncoding で変換する。
QByteArray postUrlEncode(const QString& s, Charset c);

// ---- POST body 生成 ----

// 書き込み POST body を生成する（時刻は引数注入で決定的 → 単体テスト可）。
//   jpnkn:     submit=書き込む&FROM=…&mail=…&MESSAGE=…&bbs=…&key=…&time=…
//              （移植元: BaseBBS::query()@733）
//   したらば:  submit=書き込む&NAME=…&MAIL=…&MESSAGE=…&DIR=…&BBS=…&KEY=…&TIME=…
//              （移植元: ShitarabaBBS::query()@942）
// フィールド値は board.code（Charset）で charset-urlencode する。
// submit 値は u"書き込む" を charset-urlencode した結果（移植元の化けバイトを使わない）。
QByteArray buildPostBody(const Board& board, const QString& key, BoardType type,
                         const QString& name, const QString& mail,
                         const QString& message, qint64 epochSec);

// ---- Cookie ヘッダ生成 ----

// Cookie ヘッダ値を生成する。
//   "NAME=<enc(name)>; MAIL=<enc(mail)>" + extra の各 "; <name>=<value>"
//   移植元: BaseBBS::cookie()@752（NAME/MAIL を charset-urlencode して結合）。
// extra: 2 段階 Cookie 確認の 1 回目応答で受領した Set-Cookie を渡す（再送用）。
// ※ QNAM jar と手動 Cookie ヘッダは自動マージされないため、再送時は明示合成すること。
QByteArray buildCookieHeader(const QString& name, const QString& mail, Charset c,
                             const QList<QNetworkCookie>& extra = {});

// ---- 応答判定 ----

// 書き込み応答の分類結果。
enum class WriteResult {
    Ok,         // 書き込み成功
    NeedCookie, // Cookie 確認ページ（ex0ch 系 2ch 互換: <!--2ch_X:cookie-->）
    Error,      // エラー（エラーページ / angel / jpnkn 認証 / Cookie 破損）
};

struct WriteClassification {
    WriteResult result {WriteResult::Ok};
    QString     message;    // エラー / 確認ページから抽出した説明文
};

// 書き込み POST の応答 HTML を判定して WriteClassification を返す。
// 移植元 BaseBBS::check()@387 を 3 値に拡張し jpnkn 本文エラーを追加。
//   1. halfWidthFold で全角英数→半角へ畳み込む（移植元 LCMAP_HALFWIDTH 相当）。
//   2. <!--2ch_X:(.*?)--> 内 "cookie" → NeedCookie / "error" → Error
//   3. <title>(.*?)</title> 内 "error" → Error
//      "You just summoned a Ruthless Angel." → Error
//   4. （拡張）本文に "利用認証が必要"（jpnkn Turnstile）/ "COOKIE got burnt" → Error
//   5. いずれも非該当 → Ok
WriteClassification classifyWriteResponse(const QString& decodedHtml);

// ---- 再送状態機械 ----

// 確認再送の次アクションを返す純粋状態機械。
//   Ok                                 → Succeed
//   NeedCookie && attempt==0 && cookie → Resend（高々 1 回）
//   それ以外                           → Fail
// 2 段階 Cookie は 2ch 互換掲示板（ex0ch 系）特有。したらば/jpnkn は NeedCookie を返さない。
enum class PostAction { Succeed, Resend, Fail };
PostAction nextPostAction(WriteResult r, int attempt, bool hasNewCookie);

// ---- 全角→半角畳み込み ----

// 全角 ASCII（U+FF01..FF5E）を対応する半角へ変換し、全角スペース（U+3000）を
// 半角スペースへ変換する。それ以外の文字は変換しない。
// 移植元: LCMapStr(LCMAP_HALFWIDTH) の Qt 代替。classifyWriteResponse 内で使用。
QString halfWidthFold(const QString& s);

}  // namespace yapcr::bbs
