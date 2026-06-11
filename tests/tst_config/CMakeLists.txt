qt_add_executable(tst_config
    tst_config.cpp
)

target_link_libraries(tst_config PRIVATE
    Qt6::Test
    Qt6::Core
    yapcr::config
)

add_test(NAME tst_config COMMAND tst_config)
