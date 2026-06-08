qt_add_executable(YAPCRPlayer WIN32
    src/main.cpp
    src/main_window.cpp
    src/main_window.h
)

target_link_libraries(YAPCRPlayer PRIVATE
    Qt6::Widgets
    yapcr::player
    yapcr::common
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
