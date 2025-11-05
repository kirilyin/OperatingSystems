cmake_minimum_required(VERSION 3.15)
project(Lab3 LANGUAGES CXX)

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")

add_link_options(
    $<$<CONFIG:Debug>:/NODEFAULTLIB:LIBCMTD>
    $<$<CONFIG:Release>:/NODEFAULTLIB:LIBCMT>
)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(Lab3 lab3.cpp)
target_link_libraries(Lab3 PRIVATE kernel32)

include(FetchContent)

FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.17.0.zip 
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest)

add_executable(tests tests.cpp)

target_link_libraries(tests PRIVATE gtest_main)

set_target_properties(tests PROPERTIES
    MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL"
)

enable_testing()
include(GoogleTest)
gtest_discover_tests(tests)