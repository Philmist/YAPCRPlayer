# FindMpv.cmake — scripts/fetch-mpv-dev.ps1 が用意した libmpv dev パッケージを解決する。
#
# 既定の探索先は <repo>/third_party/mpv-dev（取得スクリプトの展開先）。
# 別配置を使う場合は -DMPV_DIR=... を渡す。
#
# 定義するもの:
#   Mpv::Mpv            … IMPORTED ターゲット（include + import lib）。リンクに使う。
#   MPV_RUNTIME_DLLS    … libmpv-2.dll のフルパス（exe 隣へのコピー/配布に使う）。

if(NOT MPV_DIR)
    set(MPV_DIR "${CMAKE_SOURCE_DIR}/third_party/mpv-dev"
        CACHE PATH "libmpv dev パッケージのルート（fetch-mpv-dev.ps1 の展開先）")
endif()

find_path(MPV_INCLUDE_DIR
    NAMES mpv/client.h
    HINTS "${MPV_DIR}/include"
)

find_library(MPV_IMPLIB
    NAMES mpv libmpv
    HINTS "${MPV_DIR}"
)

find_file(MPV_RUNTIME_DLL
    NAMES libmpv-2.dll
    HINTS "${MPV_DIR}"
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Mpv
    REQUIRED_VARS MPV_INCLUDE_DIR MPV_IMPLIB
    FAIL_MESSAGE "libmpv dev が見つからない。先に scripts/fetch-mpv-dev.ps1 を実行してください。"
)

if(Mpv_FOUND AND NOT TARGET Mpv::Mpv)
    add_library(Mpv::Mpv UNKNOWN IMPORTED)
    set_target_properties(Mpv::Mpv PROPERTIES
        IMPORTED_LOCATION "${MPV_IMPLIB}"
        INTERFACE_INCLUDE_DIRECTORIES "${MPV_INCLUDE_DIR}"
    )
    set(MPV_RUNTIME_DLLS "${MPV_RUNTIME_DLL}"
        CACHE FILEPATH "配布時に同梱する libmpv-2.dll のパス")
endif()

mark_as_advanced(MPV_INCLUDE_DIR MPV_IMPLIB MPV_RUNTIME_DLL)
