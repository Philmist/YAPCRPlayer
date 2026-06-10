qt_add_executable(YAPCRPlayer WIN32
    src/main.cpp
    src/main_window.cpp
    src/main_window.h
    src/video_host_widget.cpp
    src/video_host_widget.h
    src/session_controller.cpp
    src/session_controller.h
    src/res_list_pane.cpp      # M3.6: 簡易レス一覧ペイン
    src/res_list_pane.h
    src/res_input_bar.cpp      # M3.6: 下部レス入力バー
    src/res_input_bar.h
    src/res_popup.cpp          # M3.7: hover レス・ポップアップ
    src/res_popup.h
    src/board_title_bar.cpp    # M3.7: 掲示板タイトル帯（映像下・hover トリガ）
    src/board_title_bar.h
    src/window_geometry.cpp    # M4.0: 窓サイズ純計算ヘルパ
    src/window_geometry.h
    src/display_modes.cpp      # M4.1: フィット/アスペクトモード mpv 対応付け
    src/display_modes.h
    src/snapshot_filename.cpp  # M4.4: スナップショットファイル名生成（純関数）
    src/snapshot_filename.h
)

target_link_libraries(YAPCRPlayer PRIVATE
    Qt6::Widgets
    yapcr::player
    yapcr::peercast
    yapcr::net
    yapcr::common
    yapcr::bbs              # M3.6: BbsSession + モデル（net/common/Core は推移的依存）
)

# build ツリーから直接起動できるよう libmpv-2.dll を exe の隣へ置く。
if(MPV_RUNTIME_DLLS)
    add_custom_command(TARGET YAPCRPlayer POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${MPV_RUNTIME_DLLS}" "$<TARGET_FILE_DIR:YAPCRPlayer>"
        COMMENT "libmpv-2.dll を実行ファイルの隣へコピー"
        VERBATIM
    )
endif()
