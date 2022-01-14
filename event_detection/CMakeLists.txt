cmake_minimum_required(VERSION 3.20)
set(CMAKE_CXX_STANDARD 20)

add_compile_options(-pipe -O2 -Wall)
find_package(Boost REQUIRED)
find_package(Eigen3 REQUIRED)
find_package(PythonLibs REQUIRED)

add_library(event_detection SHARED
        "tweet_stream_process.cpp"
        "sliding_window/sliding_window.cpp"
        "keyword_extraction/keyword_extraction.cpp"
        "tweet_similarity/co_occurrence_graph.cpp"
        "tweet_similarity/random_walk_with_restart.cpp"
        "tweet_similarity/similarity_handler.cpp"
        "clustering/dbscan.cpp")

set(PYTHON_INCLUDE_DIR "/usr/include/python3.8")
set(PYTHON_LIBRARIES "/usr/lib/x86_64-linux-gnu/libpython3.8.so")
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
        ${PROJECT_SOURCE_DIR}/config_handler
        ${Boost_INCLUDE_DIRS}
        ${Eigen_INCLUDE_DIRS}
        ${PYTHON_INCLUDE_DIRS}
        )

target_link_libraries(event_detection
        PUBLIC
        ${PROJECT_SOURCE_DIR}/lib/libpre_processing.so
        ${PROJECT_SOURCE_DIR}/lib/libcommon.so
        ${Boost_LIBRARIES}
        ${Eigen_LIBRARIES}
        ${PYTHON_LIBRARIES}
        )