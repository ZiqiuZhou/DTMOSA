cmake_minimum_required(VERSION 3.20)

add_compile_options(-pipe -O2 -Wall)
add_library(pre_processing SHARED
        data_parser.cpp
)

set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

target_compile_features(pre_processing
        PUBLIC
        cxx_std_20
        )

target_set_stdlib_debug_mode(pre_processing)
target_set_high_warning_level(pre_processing)
target_sanitize(pre_processing)

target_include_directories(pre_processing
        PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
        ${PROJECT_SOURCE_DIR}/include
        ${PROJECT_SOURCE_DIR}/include/rapidjson
        ${PROJECT_SOURCE_DIR}/common
)

target_link_libraries(pre_processing
        PUBLIC
        "${PROJECT_SOURCE_DIR}/lib/libcommon.so"
        )
