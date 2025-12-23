cmake_minimum_required(VERSION 3.15)
project(Lab5 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(Browser1 Browser1.cpp)
add_executable(Worker1 Worker1.cpp)

target_link_libraries(Browser1 PRIVATE kernel32)
target_link_libraries(Worker1 PRIVATE kernel32 user32)

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

add_executable(tests1 tests1.cpp)

target_link_libraries(tests1 PRIVATE gtest_main)

set_target_properties(tests1 PROPERTIES
    MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL"
)

enable_testing()
include(GoogleTest)
gtest_discover_tests(tests1)