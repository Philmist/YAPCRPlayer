#include "peercast/channel_info.h"

#include <QXmlStreamReader>

namespace yapcr::peercast {

ChannelInfo parseViewXml(const QByteArray& xml, const QString& id) {
    QXmlStreamReader reader(xml);

    while (!reader.atEnd() && !reader.hasError()) {
        if (reader.readNext() != QXmlStreamReader::StartElement) { continue; }
        if (reader.name() != QLatin1String("channel"))           { continue; }

        const auto attrs = reader.attributes();
        const QString chId = attrs.value(QLatin1String("id")).toString();
        if (QString::compare(chId, id, Qt::CaseInsensitive) != 0) {
            // 対象外 — 子要素も含めてスキップ
            reader.skipCurrentElement();
            continue;
        }

        // 対象チャンネルが見つかった
        ChannelInfo info;
        info.valid   = true;
        info.id      = chId;
        info.name    = attrs.value(QLatin1String("name")).toString();
        info.bitrate = attrs.value(QLatin1String("bitrate")).toString();
        info.type    = attrs.value(QLatin1String("type")).toString();
        info.genre   = attrs.value(QLatin1String("genre")).toString();
        info.desc    = attrs.value(QLatin1String("desc")).toString();
        info.url     = attrs.value(QLatin1String("url")).toString();
        info.uptime  = attrs.value(QLatin1String("uptime")).toString();
        info.comment = attrs.value(QLatin1String("comment")).toString();

        // 子要素 <relay> / <track> を読む（どちらも空要素 or 単一出現）
        while (reader.readNextStartElement()) {
            if (reader.name() == QLatin1String("relay")) {
                const auto ra = reader.attributes();
                bool ok = false;
                const int ls = ra.value(QLatin1String("listeners")).toInt(&ok);
                if (ok) { info.listeners = ls; }
                const int rl = ra.value(QLatin1String("relays")).toInt(&ok);
                if (ok) { info.relays = rl; }
                info.status     = ra.value(QLatin1String("status")).toString();
                info.firewalled = (ra.value(QLatin1String("firewalled")).toInt() != 0);
                reader.skipCurrentElement();
            } else if (reader.name() == QLatin1String("track")) {
                const auto ta = reader.attributes();
                info.trackTitle   = ta.value(QLatin1String("title")).toString();
                info.trackArtist  = ta.value(QLatin1String("artist")).toString();
                info.trackAlbum   = ta.value(QLatin1String("album")).toString();
                info.trackContact = ta.value(QLatin1String("contact")).toString();
                reader.skipCurrentElement();
            } else {
                reader.skipCurrentElement();
            }
        }

        return info;
    }

    return {};  // valid==false（一致チャンネルなし）
}

// ---- 表示文字列生成 --------------------------------------------------------

QString formatTitle(const ChannelInfo& info) {
    if (!info.valid || info.name.isEmpty()) { return {}; }
    if (!info.type.isEmpty()) {
        return QStringLiteral("%1 (%2)").arg(info.name, info.type);
    }
    return info.name;
}

QString formatStatus(const ChannelInfo& info) {
    if (!info.valid) { return {}; }

    QString result = formatTitle(info);

    // [genre - desc] / [genre] / [desc]
    if (!info.genre.isEmpty() || !info.desc.isEmpty()) {
        QString genreDesc;
        if (!info.genre.isEmpty() && !info.desc.isEmpty()) {
            genreDesc = QStringLiteral("%1 - %2").arg(info.genre, info.desc);
        } else if (!info.genre.isEmpty()) {
            genreDesc = info.genre;
        } else {
            genreDesc = info.desc;
        }
        result += QStringLiteral(" [%1]").arg(genreDesc);
    }

    // 「comment」
    if (!info.comment.isEmpty()) {
        result += QStringLiteral("「%1」").arg(info.comment);
    }

    // ♪artist - title（PCRPlayer の playing 文字列に相当）
    if (!info.trackTitle.isEmpty() || !info.trackArtist.isEmpty()) {
        QString track;
        if (!info.trackArtist.isEmpty() && !info.trackTitle.isEmpty()) {
            track = QStringLiteral("%1 - %2").arg(info.trackArtist, info.trackTitle);
        } else if (!info.trackTitle.isEmpty()) {
            track = info.trackTitle;
        } else {
            track = info.trackArtist;
        }
        result += QStringLiteral(" ♪%1").arg(track);
    }

    // [listeners/relays]
    if (info.listeners >= 0 && info.relays >= 0) {
        result += QStringLiteral(" [%1/%2]").arg(info.listeners).arg(info.relays);
    }

    return result;
}

}  // namespace yapcr::peercast
