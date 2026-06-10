qt_add_executable(tst_display
    tst_display.cpp
    ${CMAKE_SOURCE_DIR}/src/app/src/window_geometry.cpp
    ${CMAKE_SOURCE_DIR}/src/app/src/display_modes.cpp    # M4.1: フィット/アスペクト対応表
    ${CMAKE_SOURCE_DIR}/src/app/src/snapshot_filename.cpp  # M4.4: ファイル名生成純関数
)

target_include_directories(tst_display PRIVATE
    ${CMAKE_SOURCE_DIR}/src/app/src
)

target_link_libraries(tst_display PRIVATE
    Qt6::Test
    Qt6::Core
)

add_test(NAME tst_display COMMAND tst_display)
