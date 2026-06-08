cmake_minimum_required(VERSION 3.21)

project(YAPCRPlayer
    VERSION 0.1.0
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
# M0 は Core/Gui/Widgets のみ。M1 で Network 追加。Core5Compat は M3(BBS) で追加する。
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Network)
qt_standard_project_setup()        # AUTOMOC/AUTOUIC/AUTORCC を有効化

find_package(Mpv REQUIRED)

# --- テスト ---
enable_testing()

# --- モジュール（依存は一方向 DAG: common <- net <- peercast, common <- player, app <- 全部） ---
add_subdirectory(src/common)
add_subdirectory(src/net)
add_subdirectory(src/peercast)
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
    add_custom_target(deploy
        COMMAND ${CMAKE_COMMAND} -E make_directory "${_dist}"
        COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:YAPCRPlayer>" "${_dist}/"
        COMMAND ${CMAKE_COMMAND} -E copy "${MPV_RUNTIME_DLLS}" "${_dist}/"
        COMMAND "${WINDEPLOYQT_EXECUTABLE}" --no-translations --dir "${_dist}"
                "${_dist}/$<TARGET_FILE_NAME:YAPCRPlayer>"
        DEPENDS YAPCRPlayer
        COMMENT "windeployqt + libmpv で dist/ を作成"
        VERBATIM
    )
endif()
