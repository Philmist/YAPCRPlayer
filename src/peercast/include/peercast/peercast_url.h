#pragma once

#include <QString>

namespace yapcr::peercast {

// PeerCast /pls/ URL のパース結果。
// PCRPlayer PeerCast::PeerCast(const std::wstring& url) コンストラクタの移植。
struct PeerCastUrl {
    bool    valid{false};
    QString url;    // オリジナル URL
    QString host;   // PeerCast エンドポイントのホスト
    QString port;   // PeerCast エンドポイントのポート
    QString id;     // チャンネル ID（32 hex chars）

    // url が http://host:port/pls/<32hex> 形式かどうかを判定してパースする。
    // 有効な /pls/ URL であれば valid=true の構造体を返す。
    static PeerCastUrl parse(const QString& url);
};

}  // namespace yapcr::peercast
