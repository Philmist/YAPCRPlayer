#pragma once

#include <QByteArray>
#include <QString>

namespace yapcr::peercast {

// PeerCast viewxml の <channel> 要素（属性）＋入れ子の <relay> / <track> 属性を保持する。
// parseViewXml() の戻り値として返す。
// PCRPlayer XmlParser.h の "<xmlattr>." プレフィックスが示す通り、全フィールドが XML 属性。
struct ChannelInfo {
    bool    valid{false};

    // <channel> 属性
    QString id;
    QString name;
    QString bitrate;
    QString type;
    QString genre;
    QString desc;
    QString url;
    QString uptime;
    QString comment;

    // <relay> 属性
    int     listeners{-1};
    int     relays{-1};
    QString status;           // "RECEIVING" / "IDLE" 等
    bool    firewalled{false};

    // <track> 属性
    QString trackTitle;
    QString trackArtist;
    QString trackAlbum;
    QString trackContact;
};

// viewxml レスポンス本文 xml の中から、id（大文字小文字無視）に一致するチャンネルを返す。
// 一致するチャンネルがなければ valid==false の ChannelInfo を返す。
// QXmlStreamReader を使用（Qt6::Core のみ・追加依存なし）。
ChannelInfo parseViewXml(const QByteArray& xml, const QString& id);

// ChannelInfo からステータスバー向けの表示文字列を生成する（純関数）。
// PCRPlayer PeerCastManager::createStatus() / prepareInfo() の移植。
// 例: "チャンネル名 (FLV) [ジャンル - 説明]「コメント」 ♪アーティスト - タイトル [3/1]"
// 空フィールドは省略する。valid==false なら空文字列を返す。
QString formatStatus(const ChannelInfo& info);

// ウィンドウタイトル向けの短縮文字列（name 主体）。
// 例: "チャンネル名 (FLV)"。valid==false または name が空なら空文字列を返す。
QString formatTitle(const ChannelInfo& info);

}  // namespace yapcr::peercast
