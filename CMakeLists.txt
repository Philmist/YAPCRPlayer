cmake_minimum_required(VERSION 3.21)

project(YAPCRPlayer
    VERSION 0.2.0
    DESCRIPTION "libmpv ベースの PeerCast プレイヤー（実況用）"
    LANGUAGES CXX
)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ソースは UTF-8（日本語コメント）。MSVC は既定でシステムコードページ(CP932)解釈なので明示する。
add_compile_options($<$<CXX_COMPILER_ID:MSVC>:/utf-8>)

# 実行ファイル/DLL を build/bin にまとめる（windeployqt と libmpv 同梱を素直にするため）。
if(NOT CMAKE_RUNTIME_OUTPUT_DIRECTORY)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
endif()

# --- 依存 ---
# M0 は Core/Gui/Widgets のみ。M1 で Network 追加。
# Core5Compat は M3(BBS) で Shift-JIS/EUC-JP 変換（QTextCodec）に使用。
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Network Core5Compat OpenGLWidgets)
qt_standard_project_setup()        # AUTOMOC/AUTOUIC/AUTORCC を有効化

find_package(Mpv REQUIRED)

# 配布用に再配布可能な VC++ ランタイム(vcruntime140.dll/msvcp140.dll 等)の実体パスを得る。
# SKIP=ON で install() ルールは追加せず、変数 CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS だけ populate する。
# Release 構成では再配布可能（release）版が選ばれる。deploy で dist/ に app-local 同梱する。
set(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_SKIP ON)
include(InstallRequiredSystemLibraries)

# toml++ — TOML 永続化ライブラリ（M5.0。submodule: third_party/tomlplusplus）
# tests/example は不要なのでフラグで無効化する
set(TOMLPLUSPLUS_BUILD_TESTS    OFF CACHE BOOL "" FORCE)
set(TOMLPLUSPLUS_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(third_party/tomlplusplus)

# --- テスト ---
enable_testing()

# --- モジュール（依存は一方向 DAG: common <- net <- peercast, common <- bbs, common <- player,
#                              common <- config, app <- 全部） ---
add_subdirectory(src/common)
add_subdirectory(src/config)   # M5.0: TOML 設定基盤（tomlplusplus に依存）
add_subdirectory(src/net)
add_subdirectory(src/peercast)
add_subdirectory(src/bbs)
add_subdirectory(src/player)
add_subdirectory(src/app)
add_subdirectory(tests)

# --- 配布補助ターゲット: cmake --build --target deploy ---
# windeployqt で Qt の DLL/プラグインを集め、libmpv-2.dll を同梱した dist/ を作る。
get_target_property(_qmake Qt6::qmake IMPORTED_LOCATION)
get_filename_component(_qt_bin "${_qmake}" DIRECTORY)
find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${_qt_bin}")

if(WINDEPLOYQT_EXECUTABLE)
    set(_dist "${CMAKE_BINARY_DIR}/dist")
    # GPLv3 §4/§5/§6: 配布物に本体ライセンス・書面オファー・サードパーティ告知・
    # 各コンポーネントのライセンス全文を必ず同梱する。
    add_custom_target(deploy
        COMMAND ${CMAKE_COMMAND} -E make_directory "${_dist}"
        COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:YAPCRPlayer>" "${_dist}/"
        COMMAND ${CMAKE_COMMAND} -E copy "${MPV_RUNTIME_DLLS}" "${_dist}/"
        COMMAND "${WINDEPLOYQT_EXECUTABLE}" --no-translations --dir "${_dist}"
                "${_dist}/$<TARGET_FILE_NAME:YAPCRPlayer>"
        # --- ライセンス同梱（GPLv3 コンプライアンス） ---
        COMMAND ${CMAKE_COMMAND} -E copy
                "${CMAKE_SOURCE_DIR}/LICENSE"
                "${CMAKE_SOURCE_DIR}/CORRESPONDING-SOURCE.md"
                "${_dist}/"
        COMMAND ${CMAKE_COMMAND} -E copy_directory
                "${CMAKE_SOURCE_DIR}/licenses" "${_dist}/licenses"
        DEPENDS YAPCRPlayer
        COMMENT "windeployqt + libmpv ＋ ライセンス一式で dist/ を作成"
        VERBATIM
    )
    # 再配布可能な VC++ ランタイム DLL を app-local 同梱し、配布物を自己完結させる
    # （vc_redist インストーラ不要。Release 構成では release 版が選ばれる）。
    if(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS)
        add_custom_command(TARGET deploy POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS} "${_dist}/"
            VERBATIM
        )
    endif()

    # 実際に取得した libmpv の同定情報（fetch-mpv-dev.ps1 が生成。存在時のみ同梱）。
    if(EXISTS "${MPV_DIR}/PROVENANCE.txt")
        add_custom_command(TARGET deploy POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy
                    "${MPV_DIR}/PROVENANCE.txt" "${_dist}/licenses/libmpv-PROVENANCE.txt"
            VERBATIM
        )
    endif()
endif()
