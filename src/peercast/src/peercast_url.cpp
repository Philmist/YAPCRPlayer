#include "peercast/peercast_url.h"

#include <QRegularExpression>

namespace yapcr::peercast {

PeerCastUrl PeerCastUrl::parse(const QString& url) {
    // PCRPlayer PeerCast::PeerCast(const std::wstring& url) の正規表現を移植。
    // 原典: L"http:\\/\\/([^\\:]+):([^\\/]+)\\/pls\\/([0-9A-Za-z]{32})"
    static const QRegularExpression rx(
        QStringLiteral(R"(^http://([^:]+):([^/]+)/pls/([0-9A-Za-z]{32}))"),
        QRegularExpression::CaseInsensitiveOption);

    const auto m = rx.match(url);
    if (!m.hasMatch()) {
        return {};
    }

    PeerCastUrl result;
    result.valid = true;
    result.url   = url;
    result.host  = m.captured(1);
    result.port  = m.captured(2);
    result.id    = m.captured(3);
    return result;
}

}  // namespace yapcr::peercast
