#pragma once

// media_source.h — M5.4: クリップボード/ファイルパスの再生ソース種別判定
//
// ヘッダオンリー・純ロジック。UI / Qt Widget に依存しない（QRegularExpression/QString のみ）。
// テストは tst_shortcuts.cpp から #include するだけで検証できる。

#include <QRegularExpression>
#include <QString>

namespace yapcr::app {

// 再生ソースの種別
enum class MediaSourceKind {
    Invalid,   // 認識できない文字列（no-op）
    PlsUrl,    // PeerCast /pls/ URL（http://host:port/pls/<32hex>）
    HttpUrl,   // 一般 http(s) URL（pls でない）
    LocalPath, // ローカルファイルパス（ドライブレター / UNC / file:// スキーム）
};

// 文字列を再生ソースとして種別判定する（前後空白は trim して判定）。
// 戻り値が Invalid のとき呼び出し側は no-op にする。
inline MediaSourceKind classifyMediaSource(const QString& raw)
{
    const QString s = raw.trimmed();
    if (s.isEmpty()) return MediaSourceKind::Invalid;

    // PeerCast /pls/ URL: peercast_url.cpp の PeerCastUrl::parse と同じパターン
    //   http://host:port/pls/<32 hex chars>
    static const QRegularExpression plsRx(
        QStringLiteral(R"(^http://[^:]+:[^/]+/pls/[0-9A-Za-z]{32})"),
        QRegularExpression::CaseInsensitiveOption);
    if (plsRx.match(s).hasMatch()) return MediaSourceKind::PlsUrl;

    // 一般 http(s) URL（pls でないもの）
    static const QRegularExpression httpRx(
        QStringLiteral(R"(^https?://)"),
        QRegularExpression::CaseInsensitiveOption);
    if (httpRx.match(s).hasMatch()) return MediaSourceKind::HttpUrl;

    // ローカルパス:
    //   ドライブレター付き  C:\... または C:/...
    //   UNC パス           \\server\share\...
    //   file:// スキーム   file:///C:/...
    static const QRegularExpression pathRx(
        QStringLiteral(R"(^([A-Za-z]:[\\/]|\\\\|file://))"),
        QRegularExpression::CaseInsensitiveOption);
    if (pathRx.match(s).hasMatch()) return MediaSourceKind::LocalPath;

    return MediaSourceKind::Invalid;
}

} // namespace yapcr::app
