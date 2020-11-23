
# 编译 proto 为 .cpp 和 .h
set(PROTO_FILES proto/echo.proto)
PROTOBUF_GENERATE_CPP(PROTO_SRCS PROTO_HDRS ${PROTO_FILES})
message("PROTO_SRCS = ${PROTO_SRCS}")
message("PROTO_HDRS = ${PROTO_HDRS}")

add_executable(rpc_server_test rpc_server_test.cpp echo.pb.cc)
target_link_libraries(rpc_server_test leo)

add_executable(rpc_client_test rpc_client_test.cpp echo.pb.cc)
target_link_libraries(rpc_client_test leo) 