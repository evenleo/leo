
# 编译 proto 为 .cpp 和 .h
set(PROTO_FILES proto/args.proto)
PROTOBUF_GENERATE_CPP(PROTO_SRCS PROTO_HDRS ${PROTO_FILES})
message("PROTO_SRCS = ${PROTO_SRCS}")
message("PROTO_HDRS = ${PROTO_HDRS}")

add_executable(raft_test main.cpp raft.cpp proto/args.pb.cc)
target_link_libraries(raft_test leo)

