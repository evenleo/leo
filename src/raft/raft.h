#ifndef _RAFT_H_
#define _RAFT_H_

#include "Log.h"
#include "rpc/Rpc.h"
#include "args.h"
#include <map>
#include <memory>

using namespace leo;

struct Address {
    Address(const std::string& _ip, int _port) 
        : ip(_ip), port(_port) {}
    std::string ip;
    int port;
};

class Raft {
public:
    enum State { Follower = 0, Candidate, Leader };

    Raft(int id, int port) : running_(false), id_(id) {
        scheduler_ = std::make_shared<Scheduler>();
        scheduler_->startAsync();
        server_ = std::make_shared<RpcServer>(port, 1);
        server_->regist("add", &Raft::add, this);
        server_->regist("vote", &Raft::vote, this);
    }

    void start() {
        running_ = true;
        std::thread t([this]() {
            server_->run();
        });
        t.detach();
    }

public:
    void RescheduleElection();

		void SendRequestVote();

		void RequestVote();

		void SendAppendEntry(bool heartbeat);

		void becomeFollower(int term);

		void becomeCandidate()
        {
            ++term_;
            state_ = Candidate;
            vote_for_ = id_;
            votes_ = 1;
        }

        void becomeLeader();

		void ReceiveRequestVoteReply(const RequestVoteReply &reply) 
        {
            MutexGuard guard(mutex_);
            if (!running_ || state_ != Candidate)
                return;
            
            if (reply.term_ == term_ && reply.granted_) {
                if (++votes_ > (peers_.size() + 1) / 2)
                    becomeLeader();
            }
            else if (reply.term_ > term_)
            {
                becomeFollower(reply.term_);
            }
        }

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

    void addPeers(std::vector<Address> addresses) {
        for (auto& x : addresses) {
            if (x.port != id_) {
                peers_.push_back(std::make_shared<RpcClient>(x.ip, x.port));
            }
        }
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

    void sendRequestVote() {
        MutexGuard guard(mutex_);
        if (!running_) {
            return;
        }
        becomeCandidate();
        RequestVoteArgs args(term_, id_, 1, 1);
        for (auto& peer : peers_) {
            scheduler_->addTask([this, peer, args]() {
                response_t<RequestVoteReply> res = peer->call<RequestVoteReply>("vote", args);
                std::cout << "===========revc, " "code=" << res.code() << ", message=" << res.message() << ", value.term=" 
                    << res.value().term_ << ", value.granted_=" << res.value().granted_ << std::endl;
            });
        }
    }

private:
    std::shared_ptr<RpcServer> server_;
    std::vector<std::shared_ptr<RpcClient>> peers_;
    Scheduler::ptr scheduler_;
    Mutex mutex_;
    bool running_;

    int id_;
    int term_;
    int state_;
    int vote_for_;
    size_t votes_;
};

#endif