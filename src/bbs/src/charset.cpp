#include "bbs/charset.h"

// Shift-JIS / EUC-JP の変換を Windows API (MultiByteToWideChar/WideCharToMultiByte) で実装する。
// Qt6 Core5Compat（QTextCodec 互換）はこの Qt ビルドに未同梱のため Windows API を使用する。
// Core5Compat が利用可能な環境へ移行する際は QStringDecoder/Encoder に差し替えること。
// M5: config 化が不要な箇所だが、ファイル先頭にメモとして記載する。

#include <vector>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace yapcr::bbs {

namespace {

// Charset → Windows ANSI コードページ番号に変換する。
//   ShiftJis: CP932（Shift_JIS + Microsoft 拡張）
//   EucJp:    CP51932（Microsoft EUC-JP。IsValidCodePage=false な環境では利用不可）
UINT toCodePage(Charset c)
{
    switch (c) {
    case Charset::ShiftJis: return 932;
    case Charset::EucJp:    return 51932;
    }
    return 932;  // fallback（未到達）
}

}  // namespace

bool isCharsetSupported(Charset c)
{
    return ::IsValidCodePage(toCodePage(c)) != 0;
}

QByteArray encodeTo(const QString& s, Charset c)
{
    if (s.isEmpty()) { return {}; }
    const UINT cp = toCodePage(c);

    // 必要なバイト数を取得
    const int wlen = s.size();
    const wchar_t* wsrc = reinterpret_cast<const wchar_t*>(s.utf16());

    int bytes = WideCharToMultiByte(cp, 0, wsrc, wlen, nullptr, 0, nullptr, nullptr);
    if (bytes <= 0) { return {}; }

    QByteArray result(bytes, '\0');
    WideCharToMultiByte(cp, 0, wsrc, wlen, result.data(), bytes, nullptr, nullptr);
    return result;
}

QString decodeFrom(const QByteArray& bytes, Charset c, bool* ok)
{
    if (bytes.isEmpty()) {
        if (ok) { *ok = true; }
        return {};
    }
    const UINT cp = toCodePage(c);

    // MB_ERR_INVALID_CHARS: 無効なバイト列があれば失敗（GetLastError == ERROR_NO_UNICODE_TRANSLATION）
    int wlen = MultiByteToWideChar(
        cp, MB_ERR_INVALID_CHARS,
        bytes.constData(), bytes.size(),
        nullptr, 0);

    if (wlen <= 0) {
        if (ok) { *ok = false; }
        // エラーでも内容を返す（黙置換版で再変換）
        wlen = MultiByteToWideChar(cp, 0, bytes.constData(), bytes.size(), nullptr, 0);
        if (wlen <= 0) { return {}; }
        std::vector<wchar_t> buf(wlen);
        MultiByteToWideChar(cp, 0, bytes.constData(), bytes.size(), buf.data(), wlen);
        return QString::fromWCharArray(buf.data(), wlen);
    }

    std::vector<wchar_t> buf(wlen);
    MultiByteToWideChar(cp, MB_ERR_INVALID_CHARS,
                        bytes.constData(), bytes.size(),
                        buf.data(), wlen);
    if (ok) { *ok = true; }
    return QString::fromWCharArray(buf.data(), wlen);
}

}  // namespace yapcr::bbs
