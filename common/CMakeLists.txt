cmake_minimum_required(VERSION 3.20)
set(CMAKE_CXX_STANDARD 20)

add_library(common SHARED
        "file_io/file_io.cpp"
        "file_io/lines.cpp"
        "config_handler/config_handler.cpp"
        "geo_space/geo_space.cpp"
        )

set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

target_compile_features(common
        PUBLIC
        cxx_std_20
        )

target_set_stdlib_debug_mode(common)
target_set_high_warning_level(common)
target_sanitize(common)

target_include_directories(common
        PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
        ${Boost_INCLUDE_DIRS}
        )
