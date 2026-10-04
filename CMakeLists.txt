cmake_minimum_required(VERSION 3.21)
project(HAC_API_Test VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)

option(HAC_TESTS_ONLY "Build consumer presentation tests without the Geode SDK" OFF)
if (HAC_TESTS_ONLY)
    enable_testing()
    add_executable(hac-consumer-tests tests/StatusPresentationTest.cpp)
    target_include_directories(hac-consumer-tests PRIVATE external src)
    add_test(NAME status-presentation COMMAND hac-consumer-tests)
    return()
endif()

if (APPLE)
    if ("${CMAKE_SYSTEM_NAME}" STREQUAL "iOS" OR IOS)
        set(CMAKE_OSX_ARCHITECTURES "arm64")
    else()
        set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64")
    endif()
endif()

add_library(${PROJECT_NAME} SHARED src/main.cpp)
target_include_directories(${PROJECT_NAME} PRIVATE external)

if (NOT DEFINED ENV{GEODE_SDK})
    message(FATAL_ERROR "Set GEODE_SDK to the Geode SDK directory")
endif()
add_subdirectory($ENV{GEODE_SDK} ${CMAKE_CURRENT_BINARY_DIR}/geode)

# HAC's public API is inline and uses Geode events. This builds only the
# consumer, not the detector, and does not need HAC binary symbols. EXTERNALS
# prevents the CLI from trying to download this not-yet-indexed dependency.
# mod.json still requires the real HAC mod to be installed at runtime.
setup_geode_mod(${PROJECT_NAME} EXTERNALS "hwanhee1.hac:1.0.0")
