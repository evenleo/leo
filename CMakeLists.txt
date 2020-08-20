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
if(PROTOBUF_FOUND)
   message(STATUS "protobuf library found")
else()
    message(FATAL_ERROR "protobuf library is needed but cant be found")
endif()
find_package(ZLIB REQUIRED)
if (ZLIB_FOUND)
    include_directories(${ZLIB_INCLUDE_DIRS})
endif()

# --[ Output directory
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

file(GLOB_RECURSE srcs ${PROJECT_SOURCE_DIR}/src/*.cpp ${PROJECT_SOURCE_DIR}/src/*.c)
file(GLOB_RECURSE headers ${PROJECT_SOURCE_DIR}/src/*.h)

add_library(leo STATIC ${srcs})
target_link_libraries(leo ${CMAKE_DL_LIBS}) 	#dlsym
target_link_libraries(leo ${PROTOBUF_LIBRARIES}) #protobuf
target_link_libraries(leo ${ZLIB_LIBRARIES})	#zlib


install(TARGETS leo
        ARCHIVE DESTINATION /usr/lib
        LIBRARY DESTINATION /usr/lib)
install(FILES ${headers}
        DESTINATION /usr/local/include/leo)


add_executable(AsyncFileAppender_test test/AsyncFileAppender_test.cpp)
target_link_libraries(AsyncFileAppender_test pthread leo)

add_executable(Coroutine_test test/Coroutine_test.cpp)
target_link_libraries(Coroutine_test pthread leo)

add_executable(Log_test test/Log_test.cpp)
target_link_libraries(Log_test pthread leo)

add_executable(Timer_test test/Timer_test.cpp)
target_link_libraries(Timer_test pthread leo)

add_executable(Scheduler_test test/Scheduler_test.cpp)
target_link_libraries(Scheduler_test pthread leo)



