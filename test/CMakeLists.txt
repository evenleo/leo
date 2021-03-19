add_executable(AsyncFileAppender_test AsyncFileAppender_test.cpp)
target_link_libraries(AsyncFileAppender_test leo)

add_executable(Coroutine_test Coroutine_test.cpp)
target_link_libraries(Coroutine_test leo)

add_executable(Log_test Log_test.cpp)
target_link_libraries(Log_test leo)

add_executable(Timer_test Timer_test.cpp)
target_link_libraries(Timer_test leo)

add_executable(Scheduler_test Scheduler_test.cpp)
target_link_libraries(Scheduler_test leo)

add_executable(LogFile_test LogFile_test.cpp)
target_link_libraries(LogFile_test leo)
 
add_executable(Condition_test Condition_test.cpp)
target_link_libraries(Condition_test leo) 

add_executable(Hook_test Hook_test.cpp)
target_link_libraries(Hook_test leo) 

add_executable(HttpParser_test HttpParser_test.cpp)
target_link_libraries(HttpParser_test leo) 

add_executable(HttpRequestResponse_test HttpRequestResponse_test.cpp)
target_link_libraries(HttpRequestResponse_test leo) 

add_executable(HttpServer_test HttpServer_test.cpp)
target_link_libraries(HttpServer_test leo) 

add_executable(TcpClient_test TcpClient_test.cpp)
target_link_libraries(TcpClient_test leo) 

add_executable(TcpServer_test TcpServer_test.cpp)
target_link_libraries(TcpServer_test leo) 

add_executable(Thread_test Thread_test.cpp)
target_link_libraries(Thread_test leo) 

add_executable(EchoClient EchoClient.cpp)
target_link_libraries(EchoClient leo) 

# add_executable(serializer_test src/rpc/serializer_test.cpp)
# target_link_libraries(serializer_test leo) 
