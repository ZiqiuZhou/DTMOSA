cmake_minimum_required(VERSION 3.20)
set(CMAKE_CXX_STANDARD 20)

find_package(Boost REQUIRED COMPONENTS date_time)

add_library(event_detection SHARED
        "tweet_stream_process.cpp"
        "sliding_window/sliding_window.cpp"
        "keyword_extraction/keyword_extraction.cpp"
        "semantic_similarity/co_occurrence_graph.cpp")

set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

target_compile_features(event_detection
        PUBLIC
        cxx_std_20
        )

target_set_stdlib_debug_mode(event_detection)
target_set_high_warning_level(event_detection)
target_sanitize(event_detection)

target_include_directories(event_detection
        PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
        ${PROJECT_SOURCE_DIR}/pre_processing
        ${Boost_INCLUDE_DIRS}
        )

target_link_libraries(event_detection
        PUBLIC
        "${PROJECT_SOURCE_DIR}/lib/libpre_processing.so"
        ${Boost_LIBRARIES}
        )