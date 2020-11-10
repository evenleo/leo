#ifndef _RAFT_H_
#define _RAFT_H_

#include "Log.h"
#include "rpc/Rpc.h"
#include "args.h"
#include <map>
#include <memory>
#include <random>

using namespace leo;

struct Address
{
    Address(const std::string &_ip, int _port)
        : ip(_ip), port(_port) {}
    std::string ip;
    int port;
};

class Raft
{
public:
    enum State
    {
        Follower = 0,
        Candidate,
        Leader
    };

    const static uint64_t kTimeoutBase = 150 * 1000;
    const static uint64_t kTimeoutTop = 300 * 1000;
    const static uint64_t kElectionTimeoutBase = 300 * 1000;
    const static uint64_t kHeartbeatInterval = 30 * 1000;

    Raft(int32_t id, int port) : running_(false), id_(id), term_(0), state_(Follower)
    {
        scheduler_ = std::make_shared<Scheduler>();
        scheduler_->startAsync();
        server_ = std::make_shared<RpcServer>(port, 1);
        server_->regist("vote", &Raft::vote, this);
        server_->regist("AppendEntry", &Raft::AppendEntry, this);
    }

    void start()
    {
        running_ = true;
        std::thread t([this]() {
            server_->run();
        });
        t.detach();
    }

public:
    void rescheduleElection()
    {
        assert(state_ == Follower);
        scheduler_->cancel(election_id_);
        election_id_ = scheduler_->runAfter(getElectionTimeout(), std::make_shared<Coroutine>([this]() {
                                                std::cout << "========sendRequestVote============" << std::endl; 
                                                sendRequestVote();
                                            }));
    }

    void SendAppendEntry(bool heartbeat)
    {
        MutexGuard guard(mutex_);
        if (!running_ || state_ != Leader)
            return;
        AppendEntryArgs args = { term_, id_, 0, 1, commit_};
        for (size_t i = 0; i < peers_.size(); ++i) {
            scheduler_->addTask([this, i, args]() {
                response_t<AppendEntryReply> res = peers_[i]->call<AppendEntryReply>("AppendEntry", args);
                std::cout << "===========revc, code="
                          << res.code() << ", message=" << res.message() << ", value.term="
                          << res.value().idx_ << ", value.granted_=" << res.value().idx_ << std::endl;
            });
        } 
    }

    void becomeFollower(int term)
    {
        std::cout << "======================become Follower" << std::endl;
        if (state_ == Leader)
            scheduler_->cancel(heartbeat_id_);
        else if (state_ == Candidate)
            scheduler_->cancel(timeout_id_);
        state_ = Follower;
        term_ = term;
        vote_for_ = -1;
        rescheduleElection();
    }

    void becomeCandidate()
    {
        std::cout << "=====================become Candidate" << std::endl;
        ++term_;
        state_ = Candidate;
        vote_for_ = id_;
        votes_ = 1;
    }

    void becomeLeader()
    {
        std::cout << "=====================become Leader" << std::endl;
        state_ = Leader;
        scheduler_->cancel(timeout_id_);
        for (auto &x : nexts_)
            x = commit_ + 1;
        for (auto &x : matchs_)
            x = -1;

        heartbeat_id_ = scheduler_->runEvery(kHeartbeatInterval, std::make_shared<Coroutine>([this]() {
                                                 SendAppendEntry(true);
                                             }));
    }

    void ReceiveRequestVoteReply(const RequestVoteReply &reply)
    {
        MutexGuard guard(mutex_);
        if (!running_ || state_ != Candidate)
            return;

        if (reply.term_ == term_ && reply.granted_)
        {
            if (++votes_ > (peers_.size() + 1) / 2)
                becomeLeader();
        }
        else if (reply.term_ > term_)
        {
            becomeFollower(reply.term_);
        }
    }

    void addPeers(std::vector<Address> addresses)
    {
        MutexGuard guard(mutex_);
        for (auto &x : addresses)
        {
            if (x.port != id_)
            {
                peers_.push_back(std::make_shared<RpcClient>(x.ip, x.port));
                nexts_.push_back(0);
                matchs_.push_back(0);
            }
        }
    }

    RequestVoteReply vote(RequestVoteArgs &args)
    {
        RequestVoteReply reply;
        reply.term_ = args.term_;
        reply.granted_ = 1;
        return reply;
    }

    AppendEntryReply AppendEntry(const AppendEntryArgs& args)
    {
        MutexGuard guard(mutex_);
        AppendEntryReply reply;

        if ((args.term_ > term_) || (args.term_ == term_ && state_ == Candidate))
            becomeFollower(args.term_);
        else
            rescheduleElection();

        reply.term_ = term_;
        reply.idx_ = 1;
        return reply;
    }

    void sendRequestVote()
    {
        MutexGuard guard(mutex_);
        if (!running_)
        {
            return;
        }
        becomeCandidate();
        RequestVoteArgs args(term_, id_, 1, 1);
        for (auto &peer : peers_)
        {
            scheduler_->addTask([this, peer, args]() {
                std::cout << "begin send vote==============================================" << std::endl;
                response_t<RequestVoteReply> res = peer->call<RequestVoteReply>("vote", args);
                std::cout << "===========revc, code="
                          << res.code() << ", message=" << res.message() << ", value.term="
                          << res.value().term_ << ", value.granted_=" << res.value().granted_ << std::endl;
                ReceiveRequestVoteReply(res.value());
            });
        }

        // timeout_id_ = sh
    }

    void plantask()
    {
        timer_id_ = scheduler_->runEvery(2 * Timestamp::kMicrosecondsPerSecond,
                                         std::make_shared<Coroutine>([this]() {
                                             sendRequestVote();
                                         }));
    }

    void close()
    {
        scheduler_->cancel(timer_id_);
    }

private:
    int64_t getElectionTimeout()
    {
        static std::default_random_engine engine(time(0));
        static std::uniform_int_distribution<int64_t> dist(kTimeoutBase, kTimeoutTop);
        return dist(engine);
    }

private:
    std::shared_ptr<RpcServer> server_;
    std::vector<std::shared_ptr<RpcClient>> peers_;
    std::vector<int32_t> nexts_;
    std::vector<int32_t> matchs_;

    Scheduler::ptr scheduler_;
    Mutex mutex_;
    bool running_;

    int32_t id_;
    uint32_t term_;
    uint8_t state_;
    int32_t vote_for_;
    int32_t commit_;
    size_t votes_;

    int64_t timer_id_;
    int64_t timeout_id_;
    int64_t election_id_;
    int64_t heartbeat_id_;
};

#endif