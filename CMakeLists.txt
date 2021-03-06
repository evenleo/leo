cmake_minimum_required(VERSION 2.8)

project(leo CXX C)

set(CXX_FLAGS -O2 -g -pg -Wall -Wextra -Werror -std=c++11 -pthread)

string(REPLACE ";" " " CMAKE_CXX_FLAGS "${CXX_FLAGS}")

include_directories(${PROJECT_SOURCE_DIR}/src)
include_directories(${GMOCK_INCLUDE_DIR})

find_package(Threads REQUIRED)
find_package(Protobuf REQUIRED)

find_package(ZLIB REQUIRED)
if (ZLIB_FOUND)
    include_directories(${ZLIB_INCLUDE_DIRS})
endif()

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_subdirectory(src)
add_subdirectory(example)
add_subdirectory(raft)
add_subdirectory(test)
