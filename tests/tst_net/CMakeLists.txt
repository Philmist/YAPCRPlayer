qt_add_executable(tst_net
    tst_net.cpp
)

target_link_libraries(tst_net PRIVATE
    Qt6::Test
    yapcr::net
)

add_test(NAME tst_net COMMAND tst_net)
