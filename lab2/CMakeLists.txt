cmake_minimum_required(VERSION 3.10)

project(Lab2)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(Lab2 lab2.cpp)

target_link_libraries(Lab2 PRIVATE kernel32)
