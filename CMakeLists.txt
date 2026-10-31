cmake_minimum_required(VERSION 3.20)
project(lob CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(LOB_SANITIZE "Build with ASan + UBSan" OFF)
if(LOB_SANITIZE)
    add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer)
    add_link_options(-fsanitize=address,undefined)
endif()

add_library(lob INTERFACE)
target_include_directories(lob INTERFACE include)
target_compile_features(lob INTERFACE cxx_std_20)

include(FetchContent)
FetchContent_Declare(googletest URL https://github.com/google/googletest/archive/refs/tags/v1.15.2.tar.gz DOWNLOAD_EXTRACT_TIMESTAMP TRUE)
FetchContent_Declare(benchmark URL https://github.com/google/benchmark/archive/refs/tags/v1.9.1.tar.gz DOWNLOAD_EXTRACT_TIMESTAMP TRUE)
set(BENCHMARK_ENABLE_TESTING OFF)
set(INSTALL_GTEST OFF)
FetchContent_MakeAvailable(googletest benchmark)

enable_testing()
add_executable(tests tests/smoke_test.cpp)
target_link_libraries(tests PRIVATE lob GTest::gtest_main)
include(GoogleTest)
gtest_discover_tests(tests)

add_executable(bench bench/bench_main.cpp)
target_link_libraries(bench PRIVATE lob benchmark::benchmark_main)

foreach(t tests bench)
    target_compile_options(${t} PRIVATE -Wall -Wextra -Wpedantic -Werror)
endforeach()
