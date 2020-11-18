cmake_minimum_required(VERSION 2.8)

project(leo CXX C)

set(CXX_FLAGS
 -O2
 -g
 -pg
 -Wall
 -Wextra
 -Werror
 -std=c++11
 -pthread
)

# ---[ Flags
string(REPLACE ";" " " CMAKE_CXX_FLAGS "${CXX_FLAGS}")

# ---[ Includes
include_directories(${PROJECT_SOURCE_DIR}/src)
include_directories(${GMOCK_INCLUDE_DIR})

# ---[ Dependencies
find_package(Threads REQUIRED)
find_package(Protobuf REQUIRED)
#if(PROTOBUF_FOUND)
   #message(STATUS "protobuf library found")
#else()
    #message(FATAL_ERROR "protobuf library is needed but cant be found")
#endif()
# find_package(ZLIB REQUIRED)
# if (ZLIB_FOUND)
#     include_directories(${ZLIB_INCLUDE_DIRS})
# endif()

# --[ Output directory
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# ---[ Subdirectories
add_subdirectory(src)
# add_subdirectory(test)