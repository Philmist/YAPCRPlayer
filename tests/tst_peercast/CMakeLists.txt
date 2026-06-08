qt_add_executable(tst_peercast
    tst_peercast.cpp
)

target_link_libraries(tst_peercast PRIVATE
    Qt6::Test
    yapcr::peercast
    yapcr::common
)

add_test(NAME tst_peercast COMMAND tst_peercast)
