
add_executable(rpc_server_test rpc_server_test.cpp args.pb.cc)
target_link_libraries(rpc_server_test leo)

add_executable(rpc_client_test rpc_client_test.cpp args.pb.cc)
target_link_libraries(rpc_client_test leo) 