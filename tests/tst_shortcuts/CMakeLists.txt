qt_add_executable(tst_shortcuts
    tst_shortcuts.cpp
    ${CMAKE_SOURCE_DIR}/src/app/src/action_id.cpp
    ${CMAKE_SOURCE_DIR}/src/app/src/shortcut_keys.cpp
    ${CMAKE_SOURCE_DIR}/src/app/src/action_registry.cpp
)

target_include_directories(tst_shortcuts PRIVATE
    ${CMAKE_SOURCE_DIR}/src/app/src
)

target_link_libraries(tst_shortcuts PRIVATE
    Qt6::Test
    Qt6::Core
    Qt6::Gui     # QKeyEvent / QKeySequence は QtGui に属する
)

add_test(NAME tst_shortcuts COMMAND tst_shortcuts)
