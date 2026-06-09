#include "bbs/post.h"

#include "bbs/charset.h"

#include <QRegularExpression>
#include <QRegularExpressionMatch>

namespace yapcr::bbs {

// ---------- urlencode ----------

QByteArray postUrlEncode(const QString& s, Charset c)
{
    // 1) charset でバイト化（Shift-JIS / EUC-JP）
    const QByteArray raw = encodeTo(s, c);
    // 2) 安全集合 [0-9 a-z A-Z * - . _] でパーセントエンコード。
    //    Qt の既定安全集合は [-._~]。移植元との差分:
    //      * : 移植元は安全 → exclude に追加
    //      ~ : 移植元はエンコード対象 → include に追加
    return raw.toPercentEncoding(/*exclude=*/"*", /*include=*/"~");
}

// ---------- POST body ----------

QByteArray buildPostBody(const Board& board, const QString& key, BoardType type,
                         const QString& name, const QString& mail,
                         const QString& message, qint64 epochSec)
{
    auto enc = [&](const QString& s) -> QByteArray {
        return postUrlEncode(s, board.code);
    };

    // submit 値: u"書き込む" を charset エンコード。
    // ※移植元の L"��������" は Shift-JIS 化けバイトであり直接コピーしない。
    const QByteArray submit   = enc(QStringLiteral(u"書き込む"));
    const QByteArray timeStr  = QByteArray::number(epochSec);

    if (type == BoardType::Shitaraba) {
        // したらば: submit/NAME/MAIL/MESSAGE/DIR/BBS/KEY/TIME
        // 移植元: ShitarabaBBS::query()@942
        return QByteArrayLiteral("submit=") + submit
             + QByteArrayLiteral("&NAME=")    + enc(name)
             + QByteArrayLiteral("&MAIL=")    + enc(mail)
             + QByteArrayLiteral("&MESSAGE=") + enc(message)
             + QByteArrayLiteral("&DIR=")     + enc(board.board)
             + QByteArrayLiteral("&BBS=")     + enc(board.number)
             + QByteArrayLiteral("&KEY=")     + enc(key)
             + QByteArrayLiteral("&TIME=")    + timeStr;
    }

    // jpnkn: submit/FROM/mail/MESSAGE/bbs/key/time
    // 移植元: BaseBBS::query()@733
    return QByteArrayLiteral("submit=")    + submit
         + QByteArrayLiteral("&FROM=")     + enc(name)
         + QByteArrayLiteral("&mail=")     + enc(mail)
         + QByteArrayLiteral("&MESSAGE=")  + enc(message)
         + QByteArrayLiteral("&bbs=")      + enc(board.board)
         + QByteArrayLiteral("&key=")      + enc(key)
         + QByteArrayLiteral("&time=")     + timeStr;
}

// ---------- Cookie ヘッダ ----------

QByteArray buildCookieHeader(const QString& name, const QString& mail, Charset c,
                             const QList<QNetworkCookie>& extra)
{
    // 移植元: BaseBBS::cookie()@752
    //   return fm(L"NAME=", name) + fm(L"; MAIL=", mail);
    QByteArray result = QByteArrayLiteral("NAME=")  + postUrlEncode(name, c)
                      + QByteArrayLiteral("; MAIL=") + postUrlEncode(mail, c);

    // 再送用: 1 回目応答の Set-Cookie を合成
    for (const QNetworkCookie& ck : extra) {
        result += QByteArrayLiteral("; ") + ck.name() + "=" + ck.value();
    }
    return result;
}

// ---------- 全角→半角畳み込み ----------

QString halfWidthFold(const QString& s)
{
    QString result;
    result.reserve(s.size());
    for (const QChar ch : s) {
        const char16_t u = ch.unicode();
        if (u >= 0xFF01u && u <= 0xFF5Eu) {
            // 全角 ASCII (！..～) → 対応半角 (!..~)
            result += QChar(static_cast<char16_t>(u - 0xFEE0u));
        } else if (u == 0x3000u) {
            // 全角スペース → 半角スペース
            result += u' ';
        } else {
            result += ch;
        }
    }
    return result;
}

// ---------- 応答判定 ----------

WriteClassification classifyWriteResponse(const QString& decodedHtml)
{
    const QString half = halfWidthFold(decodedHtml);

    // 1. <!--2ch_X:(.*?)--> を探す（移植元 rx2chx / errCookie / err に相当）
    {
        static const QRegularExpression rx2chx(
            QStringLiteral("<!--\\s*2ch_X:(.*?)-->"),
            QRegularExpression::CaseInsensitiveOption);
        const QRegularExpressionMatch m = rx2chx.match(half);
        if (m.hasMatch()) {
            const QString status = m.captured(1);
            // 移植元は cookie / error を区別せず false だが、再送ロジックのため 3 値化
            if (status.contains(QStringLiteral("cookie"), Qt::CaseInsensitive)) {
                return {WriteResult::NeedCookie, status.trimmed()};
            }
            if (status.contains(QStringLiteral("error"), Qt::CaseInsensitive)) {
                return {WriteResult::Error, status.trimmed()};
            }
        }
    }

    // 2. <title>(.*?)</title>（移植元 rxTitle / err / errAngel に相当）
    {
        static const QRegularExpression rxTitle(
            QStringLiteral("<title>(.*?)</title>"),
            QRegularExpression::CaseInsensitiveOption);
        const QRegularExpressionMatch m = rxTitle.match(half);
        if (m.hasMatch()) {
            const QString status = m.captured(1);
            if (status.contains(QStringLiteral("You just summoned a Ruthless Angel."),
                                Qt::CaseInsensitive)) {
                return {WriteResult::Error, status.trimmed()};
            }
            if (status.contains(QStringLiteral("error"), Qt::CaseInsensitive)) {
                return {WriteResult::Error, status.trimmed()};
            }
        }
    }

    // 3. jpnkn 拡張: 本文テキスト検索（halfWidthFold 適用済み）
    //    - 全角 ＥＲＲＯＲ：… は halfWidthFold 後に "ERROR:…" になる
    //    - 認証要求: "この掲示板では利用認証が必要です。" はひらがなのため変換不要
    //    - Cookie 破損: "The COOKIE got burnt!" は半角のまま
    if (half.contains(QStringLiteral("利用認証が必要"), Qt::CaseSensitive)
     || half.contains(QStringLiteral("COOKIE got burnt"),  Qt::CaseInsensitive)) {
        // エラー文の先頭を message として返す
        const int pos = half.contains(QStringLiteral("利用認証が必要"), Qt::CaseSensitive)
                      ? half.indexOf(QStringLiteral("利用認証が必要"))
                      : half.indexOf(QStringLiteral("COOKIE got burnt"), 0, Qt::CaseInsensitive);
        const QString snippet = half.mid(pos, 40).trimmed();
        return {WriteResult::Error, snippet};
    }

    return {WriteResult::Ok, {}};
}

// ---------- 再送状態機械 ----------

PostAction nextPostAction(WriteResult r, int attempt, bool hasNewCookie)
{
    if (r == WriteResult::Ok) {
        return PostAction::Succeed;
    }
    if (r == WriteResult::NeedCookie && attempt == 0 && hasNewCookie) {
        // 確認ページ + 初回 + 新規 Cookie あり → 再送（高々 1 回）
        return PostAction::Resend;
    }
    return PostAction::Fail;
}

}  // namespace yapcr::bbs
