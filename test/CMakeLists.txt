add_executable(AsyncFileAppender_test AsyncFileAppender_test.cpp)
target_link_libraries(AsyncFileAppender_test ${LIBS})

add_executable(Coroutine_test Coroutine_test.cpp)
target_link_libraries(Coroutine_test ${LIBS})

add_executable(Log_test Log_test.cpp)
target_link_libraries(Log_test ${LIBS})

add_executable(Timer_test Timer_test.cpp)
target_link_libraries(Timer_test ${LIBS})

add_executable(Scheduler_test Scheduler_test.cpp)
target_link_libraries(Scheduler_test ${LIBS})

add_executable(LogFile_test LogFile_test.cpp)
target_link_libraries(LogFile_test ${LIBS})
 
add_executable(Condition_test Condition_test.cpp)
target_link_libraries(Condition_test ${LIBS}) 

add_executable(CoroutineCondition_test CoroutineCondition_test.cpp)
target_link_libraries(CoroutineCondition_test ${LIBS}) 

add_executable(Hook_test Hook_test.cpp)
target_link_libraries(Hook_test ${LIBS}) 

add_executable(HttpParser_test HttpParser_test.cpp)
target_link_libraries(HttpParser_test ${LIBS}) 

add_executable(HttpRequestResponse_test HttpRequestResponse_test.cpp)
target_link_libraries(HttpRequestResponse_test ${LIBS}) 

add_executable(HttpServer_test HttpServer_test.cpp)
target_link_libraries(HttpServer_test ${LIBS}) 

add_executable(TcpClient_test TcpClient_test.cpp)
target_link_libraries(TcpClient_test ${LIBS}) 

add_executable(TcpServer_test TcpServer_test.cpp)
target_link_libraries(TcpServer_test ${LIBS}) 

add_executable(Thread_test Thread_test.cpp)
target_link_libraries(Thread_test ${LIBS}) 

add_executable(EchoClient EchoClient.cpp)
target_link_libraries(EchoClient ${LIBS}) 

# add_executable(serializer_test src/rpc/serializer_test.cpp)
# target_link_libraries(serializer_test ${LIBS}) 
