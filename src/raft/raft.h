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

    Raft(int32_t id, int port);

    void addPeers(std::vector<Address> addresses);

    void start();

    RequestVoteReply vote(RequestVoteArgs &args);

    AppendEntryReply AppendEntry(const AppendEntryArgs &args);

    void sendRequestVote();

    void ReceiveRequestVoteReply(const RequestVoteReply &reply);

    void SendAppendEntry(bool heartbeat);

    void rescheduleElection();

    void becomeFollower(int term);

    void becomeCandidate();

    void becomeLeader();

    void close() { scheduler_->cancel(timer_id_); }

private:
    uint64_t getElectionTimeout();

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