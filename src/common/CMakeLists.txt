add_library(common STATIC
    src/version.cpp
    include/common/version.h
)
add_library(yapcr::common ALIAS common)

target_include_directories(common PUBLIC include)
target_compile_features(common PUBLIC cxx_std_20)
