#include "raft.h"
#include <functional>
#include <thread>


Raft::Raft(int32_t id, int port) : running_(false), id_(id), term_(0), state_(Follower)
{
    scheduler_ = std::make_shared<Scheduler>();
    scheduler_->startAsync();
    IpAddress addr(port);
    server_ = std::make_shared<RpcServer>(addr, scheduler_.get());
    server_->registerRpcHandler<RequestVoteArgs>(std::bind(&Raft::onRequestVote, this, std::placeholders::_1));
    server_->registerRpcHandler<RequestAppendArgs>(std::bind(&Raft::onRequestAppendEntry, this, std::placeholders::_1));
    server_->start();
}

void Raft::addPeers(std::vector<Address> addresses)
{
    MutexGuard guard(mutex_);
    for (auto &x : addresses)
    {
        if (x.port != id_)
        {
            IpAddress server_addr(x.ip, x.port);
           
            // RpcClient client(server_addr, &scheduler);
            LOG_DEBUG << "peer port=" << x.port;
            peers_.push_back(std::make_shared<RpcClient>(server_addr, scheduler_.get()));
            nexts_.push_back(0);
            matchs_.push_back(0);
        }
    }
}

void Raft::start() {
    running_ = true;
    becomeFollower(Follower);
}

// MessagePtr Raft::onRequestVote(RequestVoteArgs& args)
MessagePtr Raft::onRequestVote(std::shared_ptr<RequestVoteArgs> vote_args)
{
    LOG_DEBUG << "onRequestVote";
    std::shared_ptr<RequestVoteReply> vote_reply = std::make_shared<RequestVoteReply>();
    if (vote_args->term() < term_) {
        vote_reply->set_term(term_);
        vote_reply->set_vote_granted(false);
        LOG_DEBUG << "rejected " << vote_args->candidate_id() << " 's vote request, args.term=" << vote_args->term() << " term_=" << term_;
    } else {
        if (vote_args->term() > term_) {
            becomeFollower(vote_args->term());
        }
        vote_reply->set_term(term_);
        vote_reply->set_vote_granted(true);
    }
    return vote_reply;
}

MessagePtr Raft::onRequestAppendEntry(std::shared_ptr<RequestAppendArgs> args)
{
    MutexGuard guard(mutex_);
    std::shared_ptr<RequestAppendReply> reply = std::make_shared<RequestAppendReply>();

    if (args->term() < term_) {
        reply->set_success(false);
        reply->set_term(term_);
    } else {
        becomeFollower(args->term());
        reply->set_success(true);
        reply->set_term(term_);
    }

    return reply;
}

void Raft::sendRequestVote()
{
    MutexGuard guard(mutex_);
    if (!running_)
        return;

    becomeCandidate();
    std::shared_ptr<RequestVoteArgs> args = std::make_shared<RequestVoteArgs>();
    args->set_term(term_);
    args->set_candidate_id(id_);
    args->set_last_log_index(1);
    args->set_last_log_term(0);
    LOG_DEBUG << "sendRequestVote, term_=" << term_;
    uint64_t timeout_ms = 1;
    for (auto &peer : peers_)
    {
        scheduler_->addTask([this, peer, timeout_ms, args]() {
            // peer->setTimeout(timeout_ms);
            peer->Call<RequestVoteReply>(args, std::bind(&Raft::onRequestVoteReply, this, std::placeholders::_1));
        });
    }
}

void Raft::onRequestVoteReply(std::shared_ptr<RequestVoteReply> reply)
{
    MutexGuard guard(mutex_);
    if (!running_ || state_ != Candidate)
        return;

    if (reply->term() == term_ && reply->vote_granted()) {
        if (++votes_ > (peers_.size() + 1) / 2)
            becomeLeader();
    }
    else if (reply->term() > term_)
    {
        becomeFollower(reply->term());
    }
}

void Raft::heartbeat()
{
    MutexGuard guard(mutex_);
    if (!running_ || state_ != Leader)
        return;
    std::shared_ptr<RequestAppendArgs> args = std::make_shared<RequestAppendArgs>();
    args->set_term(term_);
    args->set_leader_id(id_);
    for (auto &peer : peers_)
    {
        scheduler_->addTask([this, peer, args]() {
            // peer->setTimeout(1);
            peer->Call<RequestAppendReply>(args, [](std::shared_ptr<RequestAppendReply> reply) {
                LOG_DEBUG << "reply->term=" << reply->term() << ", reply->success=" << reply->success();
            });
        });
    }
}

void Raft::rescheduleElection()
{
    assert(state_ == Follower);
    election_id_ = scheduler_->runAfter(getElectionTimeout(), std::make_shared<Coroutine>([this]() {
                                            std::cout << "========sendRequestVote============" << std::endl;
                                            sendRequestVote();
                                        }));
}

void Raft::becomeFollower(uint32_t term)
{
    std::cout << "======================become Follower" << std::endl;
    if (state_ == Leader)
        scheduler_->cancel(heartbeat_id_);
    else
        scheduler_->cancel(election_id_);

    state_ = Follower;
    term_ = term;
    vote_for_ = -1;
    rescheduleElection();
}

void Raft::becomeCandidate()
{
    std::cout << "=====================become Candidate" << std::endl;
    ++term_;
    state_ = Candidate;
    vote_for_ = id_;
    votes_ = 1;
}

void Raft::becomeLeader()
{
    std::cout << "=====================become Leader" << std::endl;
    state_ = Leader;
    scheduler_->cancel(timeout_id_);
    scheduler_->cancel(election_id_);
    for (auto &x : nexts_)
        x = commit_ + 1;
    for (auto &x : matchs_)
        x = -1;

    heartbeat_id_ = scheduler_->runEvery(kHeartbeatInterval, std::make_shared<Coroutine>([this]() {
                                             heartbeat();
                                         }));
}

uint64_t Raft::getElectionTimeout()
{
    static std::default_random_engine engine(time(0));
    static std::uniform_int_distribution<uint64_t> dist(kTimeoutBase, kTimeoutTop);
    return dist(engine);
}