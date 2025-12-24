cmake_minimum_required(VERSION 3.15)
project(Lab5 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(Browser1 Browser1.cpp)
add_executable(Worker1 Worker1.cpp)

target_link_libraries(Browser1 PRIVATE kernel32)
target_link_libraries(Worker1 PRIVATE kernel32 user32)