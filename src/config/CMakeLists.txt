add_library(config STATIC
    src/config_io.cpp
    include/config/config.h
)
add_library(yapcr::config ALIAS config)

target_include_directories(config PUBLIC include)

# Qt6::Core は PUBLIC（Config が QString を公開するため推移的に必要）
# tomlplusplus は実装詳細なので PRIVATE
target_link_libraries(config
    PUBLIC  Qt6::Core
    PRIVATE tomlplusplus::tomlplusplus
            yapcr::common
)

target_compile_features(config PUBLIC cxx_std_20)
