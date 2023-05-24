cmake_minimum_required(VERSION 3.16)
project(main)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Os -Wall -ggdb")

add_executable(main main.cpp basic.h pdsp.h checker.h)
