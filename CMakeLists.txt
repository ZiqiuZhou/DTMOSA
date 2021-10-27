cmake_minimum_required(VERSION 3.20)
project(GeoBurst_OSM
        LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)

option(STDLIB_DEBUG_MODE "Enable the debug mode of the standard library" OFF)
set(SANITIZERS "" CACHE STRING "List of sanitizers to enable (supported values: ASan, UBSan, ImplicitIntValueChange)")
option(TRAP_ON_UB "Generate trap instructions for undefined behavior" OFF)
option(TRAP_ON_SIGNED_OVERFLOW "Generate trap instructions for signed integer overflow" OFF)

if(TRAP_ON_UB AND NOT "UBSan" IN_LIST SANITIZERS)
    message(WARNING "Build pption `TRAP_ON_UB`: Enabling trap-on-UB has no effect unless UndefinedBehaviorSanitizer is enabled")
endif()

function(TARGET_SET_STDLIB_DEBUG_MODE TARGET)
    if(STDLIB_DEBUG_MODE)
        # MSVC and Clang support the MSVC STL. The `MSVC` variable is set by both compilers on Windows.
        if(MSVC)
            # Enable checked iterators in non-Debug builds. (Debug builds already have debug iterators enabled by default, which are a superset of checked iterators.)
            target_compile_definitions(${TARGET}
                    PRIVATE
                    "$<$<NOT:$<CONFIG:Debug>>:_ITERATOR_DEBUG_LEVEL=1>"
                    )

            # For other platforms, we handle the flags for libc++ and libstdc++. Both libraries are supported by both GCC and Clang.
        elseif(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
            # Enable libstdc++ debug mode.
            target_compile_definitions(${TARGET}
                    PRIVATE
                    "_GLIBCXX_DEBUG"
                    )

            # Enable libc++ debug mode. For Debug builds, enable iterator debugging + assertions; for Release builds, enable iterator debugging only.
            target_compile_definitions(${TARGET}
                    PRIVATE
                    "$<IF:$<CONFIG:Debug>,_LIBCPP_DEBUG=1,_LIBCPP_DEBUG=0>"
                    )

        else()
            message(WARNING "Build option `STDLIB_DEBUG_MODE`: Don't know how to enable standard libary debug mode for compiler \"${CMAKE_CXX_COMPILER_ID}\" on ${CMAKE_SYSTEM_NAME}")
        endif()
    endif()
endfunction()
function(TARGET_SET_HIGH_WARNING_LEVEL TARGET)
    if(MSVC)
        target_compile_options(${TARGET}
                PRIVATE
                /W4
                )
    elseif(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        target_compile_options(${TARGET}
                PRIVATE
                -Wall -Wextra -pedantic
                )
        # High warning levels are on by default, so don't produce a warning if we don't know how to do that for the current compiler.
    endif()
endfunction()
set(_KNOWN_SANITIZERS "ASan" "UBSan" "ImplicitIntValueChange")
foreach(_sanitizer IN LISTS SANITIZERS)
    if(NOT ${_sanitizer} IN_LIST _KNOWN_SANITIZERS)
        message(SEND_ERROR "Build setting `SANITIZERS`: unknown sanitizer '${_sanitizer}'")
    endif()
endforeach()
unset(_sanitizer)

function(TARGET_SANITIZE TARGET)
    if("ASan" IN_LIST SANITIZERS)
        if(MSVC)
            # MSVC has had support for Address Sanitizer for both x86 and x64 targets since VS 16.7:
            # https://devblogs.microsoft.com/cppblog/asan-for-windows-x64-and-debug-build-support/
            # Unfortunately it required tedious manual linking of certain runtime libraries. VS 2019 16.9 Preview 2
            # (coincidentally released just on Dec 8, 2020) finally overcame these obstacles, and all it takes now to enable ASan
            # is to pass a single flag. Therefore we won't bother trying to make ASan work for earlier versions.
            if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 19.28.29617)  # VS 2019 16.9 Preview 2
                # CMake's default options for the Debug build configuration include the "/RTC1" flag which enables a number of
                # runtime checks:
                # https://docs.microsoft.com/en-us/cpp/build/reference/rtc-run-time-error-checks
                # This flag currently isn't compatible with AddressSanitizer, cf.
                # https://developercommunity.visualstudio.com/content/problem/1144217/runtime-checks-and-address-sanitizer-are-disabled.html
                # We therefore disable runtime checks by excising the flag from the `CMAKE_CXX_FLAGS_DEBUG` variable:
                string(REGEX REPLACE "/RTC[1csu]" " " _result "${CMAKE_CXX_FLAGS_DEBUG}")
                SET(CMAKE_CXX_FLAGS_DEBUG ${_result} PARENT_SCOPE)

                # ASan is currently not compatible with the "/INCREMENTAL" linker flag, which is set by default in
                # `CMAKE_EXE_LINKER_FLAGS_<config>`, `CMAKE_SHARED_LINKER_FLAGS_<config>`, and
                # `CMAKE_MODULE_LINKER_FLAGS_<config>` for some configurations.
                foreach(_target_kind IN ITEMS "EXE" "SHARED" "MODULE")
                    foreach(_cfg_suffix IN ITEMS "" "_DEBUG" "_RELEASE" "_RELWITHDEBINFO" "_MINSIZEREL")
                        string(REGEX REPLACE "(/INCREMENTAL\\:NO|/INCREMENTAL)" "/INCREMENTAL:NO" _result "${CMAKE_${_target_kind}_LINKER_FLAGS${_cfg_suffix}}")
                        SET(CMAKE_${_target_kind}_LINKER_FLAGS${_cfg_suffix} ${_result} PARENT_SCOPE)
                    endforeach()
                endforeach()

                target_compile_options(${TARGET}
                        PRIVATE
                        "/fsanitize=address"
                        )
            elseif(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 19.27)  # VS 2019 16.7
                message(WARNING "Build setting `SANITIZERS`: don't know how to enable AddressSanitizer for MSVC versions prior to VS 2019 16.9 Preview 2")
            else()
                message(WARNING "Build setting `SANITIZERS`: AddressSanitizer is not supported by MSVC versions prior to VS 2019 16.7")
            endif()
        elseif(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
            target_compile_options(${TARGET}
                    PRIVATE
                    "-fsanitize=address"
                    )
            target_link_options(${TARGET}
                    PRIVATE
                    "-fsanitize=address"
                    )
        else()
            message(WARNING "Build setting `SANITIZERS`: don't know how to enable AddressSanitizer for compiler \"${CMAKE_CXX_COMPILER_ID}\" on ${CMAKE_SYSTEM_NAME}")
        endif()
    endif()
    if("UBSan" IN_LIST SANITIZERS)
        if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
            target_compile_options(${TARGET}
                    PRIVATE
                    "-fsanitize=undefined"
                    )
            target_link_options(${TARGET}
                    PRIVATE
                    "-fsanitize=undefined"
                    )
        else()
            message(WARNING "Build setting `SANITIZERS`: don't know how to enable UndefinedBehaviorSanitizer for compiler \"${CMAKE_CXX_COMPILER_ID}\" on ${CMAKE_SYSTEM_NAME}")
        endif()
        if(TRAP_ON_UB)
            if(CMAKE_COMPILER_IS_GNUCXX)
                target_compile_options(${TARGET}
                        PRIVATE
                        "-fsanitize-undefined-trap-on-error"
                        )
                elif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
                target_link_options(${TARGET}
                        PRIVATE
                        "-fsanitize-trap=undefined"
                        )
            endif()
        endif()
    endif()
    if("ImplicitIntValueChange" IN_LIST SANITIZERS)
        if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
            target_compile_options(${TARGET}
                    PRIVATE
                    "-fsanitize=implicit-integer-arithmetic-value-change"
                    )
        else()
            message(WARNING "Build setting `SANITIZERS`: don't know how to sanitize against `ImplicitIntValueChange` for compiler \"${CMAKE_CXX_COMPILER_ID}\" on ${CMAKE_SYSTEM_NAME}")
        endif()
    endif()
    if(TRAP_ON_SIGNED_OVERFLOW)
        if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
            target_compile_options(${TARGET}
                    PRIVATE
                    "-ftrapv"
                    )
        else()
            message(WARNING "Build setting `TRAP_ON_SIGNED_OVERFLOW`: don't know how to trap on signed integer overflow for compiler \"${CMAKE_CXX_COMPILER_ID}\" on ${CMAKE_SYSTEM_NAME}")
        endif()
    endif()
endfunction()

include_directories(

)

add_executable(GeoBurst_OSM main.cpp)

target_compile_features(GeoBurst_OSM
        PRIVATE
        cxx_std_20
        )
target_set_stdlib_debug_mode(GeoBurst_OSM)
target_set_high_warning_level(GeoBurst_OSM)
target_sanitize(GeoBurst_OSM)
set_target_properties(GeoBurst_OSM
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}
        LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}
        )

set(BOOST_ROOT /usr/include/boost)
find_package(Boost)

add_subdirectory("common")
add_subdirectory("pre_processing")
add_subdirectory("event_detection")

target_link_libraries(GeoBurst_OSM
        PRIVATE
        common
        pre_processing
        event_detection)

enable_testing()

