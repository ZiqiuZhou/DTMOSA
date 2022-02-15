cmake_minimum_required(VERSION 3.20)
set(CMAKE_CXX_STANDARD 20)

add_compile_options(-pipe -O2 -Wall)

add_library(osm_semantic_annotation SHARED
        osm_annotation_process.cpp
        spatial_data_integration.cpp
        semantic_annotation.cpp)

set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

target_compile_features(osm_semantic_annotation
        PUBLIC
        cxx_std_20
        )

target_set_stdlib_debug_mode(osm_semantic_annotation)
target_set_high_warning_level(osm_semantic_annotation)
target_sanitize(osm_semantic_annotation)

target_include_directories(osm_semantic_annotation
        PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
        ${PROJECT_SOURCE_DIR}/pre_processing
        ${PROJECT_SOURCE_DIR}/config_handler
        ${PROJECT_SOURCE_DIR}/event_detection
        )

target_link_libraries(osm_semantic_annotation
        PUBLIC
        ${PROJECT_SOURCE_DIR}/lib/libpre_processing.so
        ${PROJECT_SOURCE_DIR}/lib/libcommon.so
        ${PROJECT_SOURCE_DIR}/lib/libevent_detection.so
        )