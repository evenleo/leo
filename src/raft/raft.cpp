#include "raft.h"

Raft::Raft(int32_t id, int port) : running_(false), id_(id), term_(0), state_(Follower)
{
    scheduler_ = std::make_shared<Scheduler>();
    scheduler_->startAsync();
    server_ = std::make_shared<RpcServer>(port, 1);
    server_->regist("vote", &Raft::vote, this);
    server_->regist("AppendEntry", &Raft::AppendEntry, this);
}

void Raft::addPeers(std::vector<Address> addresses)
{
    MutexGuard guard(mutex_);
    for (auto &x : addresses)
    {
        if (x.port != id_)
        {
            LOG_DEBUG << "peer port=" << x.port;
            peers_.push_back(std::make_shared<RpcClient>(x.ip, x.port));
            nexts_.push_back(0);
            matchs_.push_back(0);
        }
    }
}

void Raft::start()
{
    running_ = true;
    std::thread t([this]() {
        server_->run();
    });
    t.detach();
}

RequestVoteReply Raft::vote(RequestVoteArgs &args)
{
    RequestVoteReply reply;
    if (args.term_ > term_)
        becomeFollower(args.term_);
    reply.term_ = args.term_;
    reply.granted_ = 1;
    return reply;
}

AppendEntryReply Raft::AppendEntry(const AppendEntryArgs &args)
{
    MutexGuard guard(mutex_);
    AppendEntryReply reply;

    LOG_DEBUG << "term_=" << term_ << ", args.term_=" << args.term_;
    if ((args.term_ > term_) || (args.term_ == term_ && state_ == Candidate))
        becomeFollower(args.term_);
    else
        rescheduleElection();

    reply.term_ = term_;
    reply.idx_ = 1;
    return reply;
}

void Raft::sendRequestVote()
{
    MutexGuard guard(mutex_);
    if (!running_)
        return;

    becomeCandidate();
    RequestVoteArgs args(term_, id_, 1, 1);
    LOG_DEBUG << "sendRequestVote, term_=" << term_;
    uint64_t timeout_ms = (kTimeoutBase + getElectionTimeout()) / 1000;
    for (auto &peer : peers_)
    {
        scheduler_->addTask([this, peer, timeout_ms, args]() {
            peer->setTimeout(timeout_ms);
            response_t<RequestVoteReply> res = peer->call<RequestVoteReply>("vote", args);
            if (res.code() == RPC_ERR_SUCCESS)
            {
                LOG_DEBUG << "rpc request response, code=" << res.code() << ", message=" << res.message()
                          << ", value.term=" << res.value().term_ << ", value.granted_=" << res.value().granted_;
                ReceiveRequestVoteReply(res.value());
            }
            else
            {
                LOG_DEBUG << "rpc request timeout!";
            }
        });
    }

    // timeout_id_ = sh
}

void Raft::ReceiveRequestVoteReply(const RequestVoteReply &reply)
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

void Raft::SendAppendEntry(bool heartbeat)
{
    MutexGuard guard(mutex_);
    if (!running_ || state_ != Leader)
        return;
    AppendEntryArgs args = {term_, id_, 0, 1, commit_};
    for (size_t i = 0; i < peers_.size(); ++i)
    {
        scheduler_->addTask([this, i, args]() {
            response_t<AppendEntryReply> res = peers_[i]->call<AppendEntryReply>("AppendEntry", args);
            std::cout << "===========revc, code="
                      << res.code() << ", message=" << res.message() << ", value.term="
                      << res.value().idx_ << ", value.granted_=" << res.value().idx_ << std::endl;
        });
    }
}

void Raft::rescheduleElection()
{
    assert(state_ == Follower);
    scheduler_->cancel(election_id_);
    election_id_ = scheduler_->runAfter(getElectionTimeout(), std::make_shared<Coroutine>([this]() {
                                            std::cout << "========sendRequestVote============" << std::endl;
                                            sendRequestVote();
                                        }));
}

void Raft::becomeFollower(int term)
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
    for (auto &x : nexts_)
        x = commit_ + 1;
    for (auto &x : matchs_)
        x = -1;

    heartbeat_id_ = scheduler_->runEvery(kHeartbeatInterval, std::make_shared<Coroutine>([this]() {
                                             SendAppendEntry(true);
                                         }));
}

uint64_t Raft::getElectionTimeout()
{
    static std::default_random_engine engine(time(0));
    static std::uniform_int_distribution<uint64_t> dist(kTimeoutBase, kTimeoutTop);
    return dist(engine);
}