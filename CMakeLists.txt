cmake_minimum_required(VERSION 3.0)

project(test VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(test main.cpp)
add_executable(create create.cpp)
add_executable(save save.cpp)
add_executable(restore restore.cpp)
add_executable(delete delete.cpp)


