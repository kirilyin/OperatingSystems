cmake_minimum_required(VERSION 3.10)

project(Lab3)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(Lab3 lab3.cpp)

target_link_libraries(Lab3 PRIVATE kernel32)
