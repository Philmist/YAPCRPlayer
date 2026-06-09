qt_add_executable(tst_bbs
    tst_bbs.cpp
)

target_link_libraries(tst_bbs PRIVATE
    Qt6::Test
    yapcr::bbs
)

add_test(NAME tst_bbs COMMAND tst_bbs)
