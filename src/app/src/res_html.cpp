#include "res_html.h"

#include "bbs/extract.h"

#include <QRegularExpression>
#include <QStringList>

namespace yapcr::app::reshtml {

namespace {

// タグを除去する。keepBr=true なら <br>（<br/> / <br /> 含む）は残し改行として描画させる。
// HTML エンティティ（&gt; など）はデコードせずそのまま残す（Qt リッチテキスト用）。
QString stripTags(const QString& html, bool keepBr)
{
    QString s = html;
    static const QRegularExpression rxBr(
        QStringLiteral(R"(<\s*br\s*/?\s*>)"),
        QRegularExpression::CaseInsensitiveOption);
    if (keepBr) {
        // 一旦保護文字へ退避してから他タグを除去し、最後に <br> へ復帰させる
        s.replace(rxBr, QStringLiteral("\x01"));
    }
    static const QRegularExpression rxTag(QStringLiteral(R"(<[^>]*>)"));
    s.remove(rxTag);
    if (keepBr) {
        s.replace(QChar(u'\x01'), QStringLiteral("<br>"));
    }
    return s;
}

// 本文中のアンカー区間を着色（リンク or span）で包む。
// 渡す text は stripTags 済み（&gt; エンティティは残存）であること。
QString wrapAnchors(const QString& text, bool asLinks)
{
    const QList<yapcr::bbs::AnchorSpan> spans = yapcr::bbs::extractAnchorSpans(text);
    if (spans.isEmpty()) {
        return text;
    }

    QString out;
    int pos = 0;
    for (const yapcr::bbs::AnchorSpan& sp : spans) {
        out += text.mid(pos, sp.start - pos);
        const QString shown = text.mid(sp.start, sp.length);
        if (asLinks) {
            out += QStringLiteral("<a href=\"range:%1-%2\" style=\"color:%3;\">%4</a>")
                       .arg(QString::number(sp.range.first),
                            QString::number(sp.range.last),
                            kAnchorColor,
                            shown);
        } else {
            out += QStringLiteral("<span style=\"color:%1;\">%2</span>")
                       .arg(kAnchorColor, shown);
        }
        pos = sp.start + sp.length;
    }
    out += text.mid(pos);
    return out;
}

}  // namespace

QString resToHtml(const yapcr::bbs::ResInfo& r, bool withAnchorLinks)
{
    const bool sage = (r.mail.compare(QLatin1String("sage"), Qt::CaseInsensitive) == 0);
    // 名前色: age（メール非空かつ sage 以外）= 赤 / それ以外（sage・メール空）= 緑
    const QString& nameColor = (!r.mail.isEmpty() && !sage) ? kNameAgeColor : kNameSageColor;

    QStringList parts;

    // レス番（withAnchorLinks 時はホバー用 <a href="ref:N">）
    if (!r.number.isEmpty()) {
        if (withAnchorLinks) {
            parts << QStringLiteral("<a href=\"ref:%1\" style=\"color:%2;\">%1</a>")
                         .arg(r.number, kNumberColor);
        } else {
            parts << QStringLiteral("<span style=\"color:%1;\">%2</span>")
                         .arg(kNumberColor, r.number);
        }
    }

    // 名前（pre-escaped HTML のためタグ除去のみ・エンティティは残す）
    const QString name = stripTags(r.name, /*keepBr=*/false);
    if (!name.isEmpty()) {
        parts << QStringLiteral("<b><span style=\"color:%1;\">%2</span></b>")
                     .arg(nameColor, name);
    }

    // メール欄（非空時のみ [mail] を青で表示）
    const QString mail = stripTags(r.mail, /*keepBr=*/false);
    if (!mail.isEmpty()) {
        parts << QStringLiteral("<span style=\"color:%1;\">[%2]</span>")
                     .arg(kMailColor, mail);
    }

    // 日時＋ID（プレーンテキストなので HTML エスケープして埋め込む）
    QString meta = r.datetime;
    if (!r.id.isEmpty()) {
        if (!meta.isEmpty()) { meta += QLatin1Char(' '); }
        meta += QStringLiteral("ID:") + r.id;
    }
    if (!meta.isEmpty()) {
        parts << QStringLiteral("<span style=\"color:%1;\">%2</span>")
                     .arg(kTimeColor, meta.toHtmlEscaped());
    }

    const QString header = parts.join(QLatin1Char(' '));
    const QString body   = wrapAnchors(stripTags(r.message, /*keepBr=*/true), withAnchorLinks);

    return QStringLiteral("<div>%1<br>%2</div>").arg(header, body);
}

}  // namespace yapcr::app::reshtml
