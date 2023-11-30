cmake_minimum_required(VERSION 3.16)
project(main)

set(CMAKE_CXX_STANDARD 17)
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Ofast -Wall -Werror")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -ggdb -fsanitize=address")

include_directories(include)

add_library(pds_shared SHARED src/heap.cpp src/graph.cpp src/pds.cpp)

add_executable(main src/main.cpp)
add_executable(checker src/checker.cpp)

target_link_libraries(main pds_shared)
target_link_libraries(checker pds_shared)
