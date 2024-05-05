cmake_minimum_required(VERSION 3.16)
project(main)
list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

set(CMAKE_CXX_STANDARD 20)
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Ofast -Wall -Werror")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -O0 -ggdb -fsanitize=address")

include_directories(include)

add_library(pdslib SHARED src/nupds.cpp src/pdsgraph.cpp)

add_executable(main src/main.cpp)
target_link_libraries(main PUBLIC pdslib)

add_executable(test src/test.cpp)
target_link_libraries(test PUBLIC pdslib)
