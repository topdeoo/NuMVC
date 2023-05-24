cmake_minimum_required(VERSION 3.16)
project(PDSP)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Os -Wall")

add_executable(PDSP main.cpp basic.h pdsp.h checker.h)
