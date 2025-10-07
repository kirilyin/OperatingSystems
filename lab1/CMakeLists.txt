cmake_minimum_required(VERSION 3.10)

project(Lab1)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(Lab1 lab1.cpp)

target_link_libraries(Lab1 PRIVATE kernel32)
