qt_add_executable(tst_bbs
    tst_bbs.cpp
)

target_link_libraries(tst_bbs PRIVATE
    Qt6::Test
    yapcr::bbs
)

# フィクスチャの絶対パスをコンパイル定義で渡す（QFile でファイルを直接開くため）
target_compile_definitions(tst_bbs PRIVATE
    BBS_FIXTURES_DIR="${CMAKE_CURRENT_SOURCE_DIR}/../fixtures/bbs"
)

add_test(NAME tst_bbs COMMAND tst_bbs)
