add_library(player STATIC
    src/mpv_backend.cpp
    include/player/mpv_backend.h
)
add_library(yapcr::player ALIAS player)

target_include_directories(player PUBLIC include)
target_link_libraries(player
    PUBLIC  yapcr::common
            Qt6::Core      # MpvBackend は QObject（ヘッダが QObject を公開するため PUBLIC）
    PRIVATE Mpv::Mpv
)
target_compile_features(player PUBLIC cxx_std_20)
