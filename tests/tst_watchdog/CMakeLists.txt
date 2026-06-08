qt_add_executable(tst_watchdog
    tst_watchdog.cpp
)

target_link_libraries(tst_watchdog PRIVATE
    Qt6::Test
    yapcr::player
    yapcr::common
)

add_test(NAME tst_watchdog COMMAND tst_watchdog)
