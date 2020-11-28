#ifndef _RAFT_H_
#define _RAFT_H_

#include "Log.h"
#include "rpc/RpcServer.h"
#include "rpc/RpcClient.h"
#include "args.pb.h"
#include <google/protobuf/message.h>
#include <map>
#include <memory>
#include <random>

using namespace leo;
using namespace leo::rpc;



class Raft
{
public:
    enum State
    {
        Follower = 0,
        Candidate,
        Leader
    };

    const static uint64_t kTimeoutBase = 150 * 1000 * 10;
    const static uint64_t kTimeoutTop = 300 * 1000 * 10;
    const static uint64_t kElectionTimeoutBase = 300 * 1000 * 10;
    const static uint64_t kHeartbeatInterval = 100 * 1000 * 10;

    Raft(uint32_t me, int port, Scheduler::ptr scheduler, std::vector<RpcClient::ptr>& peers);

    // void addPeers(std::vector<Address> addresses);
    
    void start();

private:
    MessagePtr onRequestVote(std::shared_ptr<RequestVoteArgs> vote_args);

    MessagePtr onRequestAppendEntry(std::shared_ptr<RequestAppendArgs> args);

    void sendRequestVote();

    void onRequestVoteReply(std::shared_ptr<RequestVoteReply> reply);

    void onRequestAppendReply(uint32_t target_server, 
                              std::shared_ptr<RequestAppendArgs> append_args, 
                              std::shared_ptr<RequestAppendReply> reply);

    void heartbeat();

    void packEntrys(size_t next_index, std::shared_ptr<RequestAppendArgs> append_args);

    void becomeFollower(uint32_t term);

    void becomeCandidate();

    void becomeLeader();

    void close() {}

    uint32_t getLastEntryIndex() const { return log_.size() - 1; }

    bool upToDate(uint32_t last_log_index, uint32_t last_log_term) const;

    void resetLeaderState();

    uint64_t getElectionTimeout();

private:
    uint32_t me_;
    uint8_t state_;
    
    //persistent state on all servers
    uint32_t current_term_;
    int32_t vote_for_;
    size_t votes_;
	std::vector<LogEntry> log_;

    //volatile state on all servers
    uint32_t commit_index_;
	uint32_t last_applied_;

    //valotile state on leaders
    std::vector<int32_t> next_index_;
    std::vector<int32_t> match_index_;

    Mutex mutex_;
    std::atomic_bool running_;
    Scheduler::ptr scheduler_;
    RpcServer::ptr server_;
    std::vector<RpcClient::ptr> peers_;
 
    int64_t timeout_id_;
    int64_t heartbeat_id_;
};

#endif