cmake_minimum_required(VERSION 3.10)

project(Lab1)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(Lab1 lab1.cpp "common.h" "common.cpp")
add_executable(Lab1_9 lab1task9.cpp "common.h" "common.cpp")

target_link_libraries(Lab1 PRIVATE kernel32)
target_link_libraries(Lab1_9 PRIVATE kernel32)


set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")

add_link_options(
    $<$<CONFIG:Debug>:/NODEFAULTLIB:LIBCMTD>
    $<$<CONFIG:Release>:/NODEFAULTLIB:LIBCMT>
)

include(FetchContent)

FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip 
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest)

add_executable(tests2 tests.cpp "common.h" "common.cpp")

target_link_libraries(tests2 PRIVATE gtest_main)

set_target_properties(tests2 PROPERTIES
    MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL"
)

enable_testing()
include(GoogleTest)
gtest_discover_tests(tests2)