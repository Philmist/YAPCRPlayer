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
            Qt6::Core5Compat   # QTextCodec（Shift-JIS/EUC-JP）
)
target_compile_features(bbs PUBLIC cxx_std_20)
