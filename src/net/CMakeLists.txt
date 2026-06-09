add_library(net STATIC
    src/http_message.cpp
    src/http_client.cpp
    include/net/http_message.h
    include/net/http_client.h
)
add_library(yapcr::net ALIAS net)

target_include_directories(net PUBLIC include)
target_link_libraries(net
    PUBLIC  yapcr::common
            Qt6::Core
            Qt6::Network
)
target_compile_features(net PUBLIC cxx_std_20)
