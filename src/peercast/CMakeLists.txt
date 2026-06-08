add_library(peercast STATIC
    src/peercast_url.cpp
    src/stream_resolver.cpp
    src/channel_info.cpp
    src/peercast_controller.cpp
    include/peercast/peercast_url.h
    include/peercast/stream_resolver.h
    include/peercast/channel_info.h
    include/peercast/peercast_controller.h
)
add_library(yapcr::peercast ALIAS peercast)

target_include_directories(peercast PUBLIC include)
target_link_libraries(peercast
    PUBLIC  yapcr::common
            yapcr::net
            Qt6::Core
)
target_compile_features(peercast PUBLIC cxx_std_20)
