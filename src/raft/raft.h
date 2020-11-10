#ifndef _RAFT_H_
#define _RAFT_H_

#include "Log.h"
#include "rpc/Rpc.h"
#include "args.h"
#include <map>
#include <memory>

using namespace leo;

class Raft {
public:
    enum State { Follower = 0, Candidate, Leader };

    Raft(int id) {
        server_ = std::make_shared<RpcServer>(5001, 1);
        server_->regist("add", &Raft::add, this);
        server_->regist("vote", &Raft::vote, this);
    }

    void start() {
        std::thread t([this]() {
            server_->run();
        });
        t.detach();
    }

public:
    void peer(const std::string& ip, int port) {
        std::cout << "peer" << std::endl;
        RpcClient client(ip, port);
        response_t<int> res = client.call<int>("add", 10, 21);
        std::cout << "code=" << res.code() << ", message=" << res.message() << ", value=" << res.value() << std::endl;
    }

    void voteTest(const std::string& ip, int port) {
        std::cout << "peer" << std::endl;
        RpcClient client(ip, port);
        RequestVoteArgs args(10, 1, 1, 1);
        response_t<RequestVoteReply> res = client.call<RequestVoteReply>("vote", args);
        std::cout << "code=" << res.code() << ", message=" << res.message() << ", value.term=" 
        << res.value().term_ << ", value.granted_=" << res.value().granted_ << std::endl;
    }

    int add(int a, int b) {
        return a + b;
    }

    RequestVoteReply vote(RequestVoteArgs& args) {
        RequestVoteReply reply;
        reply.term_ = args.term_ + 1;
        reply.granted_ = 1;
        return reply;
    }

private:
    int id_;
    std::shared_ptr<RpcServer> server_;
    std::map<int, std::shared_ptr<RpcClient>> peers_;
};

#endif