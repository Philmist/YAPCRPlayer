#pragma once

namespace yapcr::player {

// libmpv クライアント API のバージョン（mpv_client_api_version() の戻り値）。
// 上位16bit がメジャー、下位16bit がマイナー。
// M0 では「libmpv がリンクでき、ランタイム DLL がロードできる」ことの実証にのみ使う。
// M1 以降でこのヘッダに MpvBackend（生成/--wid アタッチ/loadfile/プロパティ）を実装する。
unsigned long mpvClientApiVersion();

}  // namespace yapcr::player
