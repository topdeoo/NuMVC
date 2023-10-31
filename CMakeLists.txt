cmake_minimum_required(VERSION 3.16)
project(main)

set(CMAKE_CXX_STANDARD 17)
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -Wall -Werror")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -ggdb -fsanitize=address")

include_directories(include)

add_library(pds_shared SHARED src/heap.cpp src/graph.cpp src/pds.cpp)

add_executable(main src/main.cpp)

target_link_libraries(main pds_shared)
