add_library(bbs STATIC
    src/charset.cpp
    src/board_url.cpp
    src/setting.cpp
    src/subject.cpp
    src/dat.cpp
    src/extract.cpp
    src/dat_store.cpp
    src/post.cpp
    src/bbs_session.cpp
    include/bbs/models.h
    include/bbs/charset.h
    include/bbs/board_url.h
    include/bbs/setting.h
    include/bbs/subject.h
    include/bbs/dat.h
    include/bbs/extract.h
    include/bbs/dat_store.h
    include/bbs/post.h
    include/bbs/bbs_session.h
)
add_library(yapcr::bbs ALIAS bbs)

target_include_directories(bbs PUBLIC include)
target_link_libraries(bbs
    PUBLIC  yapcr::common
            yapcr::net          # BbsSession が HttpClient を使用
            Qt6::Core
            Qt6::Core5Compat   # QTextCodec（Shift-JIS/EUC-JP）
)
target_compile_features(bbs PUBLIC cxx_std_20)
