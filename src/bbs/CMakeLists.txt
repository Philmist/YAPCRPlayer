add_library(bbs STATIC
    src/charset.cpp
    src/board_url.cpp
    include/bbs/models.h
    include/bbs/charset.h
    include/bbs/board_url.h
)
add_library(yapcr::bbs ALIAS bbs)

target_include_directories(bbs PUBLIC include)
target_link_libraries(bbs
    PUBLIC  yapcr::common
            Qt6::Core
    # Qt6::Core5Compat は Shift-JIS/EUC-JP の名前指定コーデックに使う予定だったが、
    # この Qt 6.11.1 ビルドには Core5Compat が同梱されていないため Windows API で代替する。
    # 将来 Core5Compat が利用可能になったときに charset.cpp を差し替える。
)
target_compile_features(bbs PUBLIC cxx_std_20)
