cmake_minimum_required(VERSION 2.8)

project(leo CXX C)

cmake_minimum_required(VERSION 3.0)
project(leo)

#set(CMAKE_VERBOSE_MAKEFILE ON)
#set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -rdynamic -O3 -fPIC -ggdb -std=c++14 -Wall -Wno-deprecated -Werror -Wno-unused-function -Wno-builtin-macro-redefined -Wno-deprecated-declarations")

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -rdynamic -O3 -fPIC -ggdb -std=c++14 -Wall -Wno-deprecated -Werror -Wno-unused-function -Wno-builtin-macro-redefined -Wno-deprecated-declarations")
set(CMAKE_C_FLAGS "$ENV{CXXFLAGS} -rdynamic -O3 -fPIC -ggdb -std=c11 -Wall -Wno-deprecated -Werror -Wno-unused-function -Wno-builtin-macro-redefined -Wno-deprecated-declarations")

include_directories(src)

set(LIB_SRC
    src/Address.cpp
    src/Buffer.cpp
    src/Condition.cpp
    src/Coroutine.cpp
    src/CountDownLatch.cpp
    src/Hook.cpp
    src/Log.cpp
    src/LogFile.cpp
    src/Mutex.cpp
    src/Poller.cpp
    src/EventPoller.cpp
    src/Processer.cpp
    src/ProcesserThread.cpp
    src/Scheduler.cpp
    src/Socket.cpp
    src/TcpClient.cpp
    src/TcpConnection.cpp
    src/TcpServer.cpp
    src/Thread.cpp
    src/TimerManager.cpp
    src/Timestamp.cpp

    src/http/picohttpparser.c
    src/http/Http.cpp
    src/http/HttpParser.cpp
    src/http/HttpConnection.cpp
)

add_library(leo SHARED ${LIB_SRC})

set(LIBS
    leo
    pthread
    dl
)

add_executable(AsyncFileAppender_test test/AsyncFileAppender_test.cpp)
target_link_libraries(AsyncFileAppender_test ${LIBS})

add_executable(Coroutine_test test/Coroutine_test.cpp)
target_link_libraries(Coroutine_test ${LIBS})

add_executable(Log_test test/Log_test.cpp)
target_link_libraries(Log_test ${LIBS})

add_executable(Timer_test test/Timer_test.cpp)
target_link_libraries(Timer_test ${LIBS})

add_executable(Scheduler_test test/Scheduler_test.cpp)
target_link_libraries(Scheduler_test ${LIBS})

add_executable(LogFile_test test/LogFile_test.cpp)
target_link_libraries(LogFile_test ${LIBS})
 
add_executable(Condition_test test/Condition_test.cpp)
target_link_libraries(Condition_test ${LIBS}) 

add_executable(CoroutineCondition_test test/CoroutineCondition_test.cpp)
target_link_libraries(CoroutineCondition_test ${LIBS}) 

add_executable(Hook_test test/Hook_test.cpp)
target_link_libraries(Hook_test ${LIBS}) 

add_executable(HttpParser_test test/HttpParser_test.cpp)
target_link_libraries(HttpParser_test ${LIBS}) 

add_executable(HttpRequestResponse_test test/HttpRequestResponse_test.cpp)
target_link_libraries(HttpRequestResponse_test ${LIBS}) 

add_executable(HttpServer_test test/HttpServer_test.cpp)
target_link_libraries(HttpServer_test ${LIBS}) 

add_executable(TcpClient_test test/TcpClient_test.cpp)
target_link_libraries(TcpClient_test ${LIBS}) 

add_executable(TcpServer_test test/TcpServer_test.cpp)
target_link_libraries(TcpServer_test ${LIBS}) 

add_executable(Thread_test test/Thread_test.cpp)
target_link_libraries(Thread_test ${LIBS}) 

add_executable(EchoClient test/EchoClient.cpp)
target_link_libraries(EchoClient ${LIBS}) 

add_executable(serializer_test src/rpc/test/serializer_test.cpp)
target_link_libraries(serializer_test ${LIBS}) 

add_executable(rpc_server_test src/rpc/test/rpc_server_test.cpp)
target_link_libraries(rpc_server_test ${LIBS}) 

add_executable(rpc_client_test src/rpc/test/rpc_client_test.cpp)
target_link_libraries(rpc_client_test ${LIBS}) 
