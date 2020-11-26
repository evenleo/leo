#include "raft.h"
#include <functional>
#include <thread>


Raft::Raft(int32_t id, int port) 
    : id_(id),
      state_(Follower),
      current_term_(0),
      vote_for_(-1),
      running_(false)
{
    scheduler_ = std::make_shared<Scheduler>();
    scheduler_->startAsync();
    IpAddress addr(port);
    server_ = std::make_shared<RpcServer>(addr, scheduler_.get());
    server_->registerHandler<RequestVoteArgs>(std::bind(&Raft::onRequestVote, this, std::placeholders::_1));
    server_->registerHandler<RequestAppendArgs>(std::bind(&Raft::onRequestAppendEntry, this, std::placeholders::_1));
    server_->start();
    
    //Figure 2表明logs下标从1开始，所以添加一个空的Entry
    LogEntry entry;
    entry.set_term(0);
    entry.set_index(0);
    log_.push_back(entry);
}

void Raft::addPeers(std::vector<Address> addresses)
{
    MutexGuard guard(mutex_);
    for (auto &x : addresses)
    {
        if (x.port != id_)
        {
            IpAddress server_addr(x.ip, x.port);
            LOG_DEBUG << "peer port=" << x.port;
            peers_.push_back(std::make_shared<RpcClient>(server_addr, scheduler_.get()));
        }
    }
}

void Raft::start() {
    running_ = true;
    if (vote_for_ == -1)
        becomeFollower(Follower);
}

/**
 * 请求投票 RPC
 *
 * 接收者实现:
 *      如果term < currentTerm返回 false （5.2 节）
 *      如果 votedFor 为空或者就是 candidateId，并且候选人的日志至少和自己一样新，那么就投票给他（5.2 节，5.4 节）
 */
MessagePtr Raft::onRequestVote(std::shared_ptr<RequestVoteArgs> vote_args)
{
    LOG_DEBUG << "onRequestVote";
    MutexGuard guard(mutex_);
    std::shared_ptr<RequestVoteReply> vote_reply = std::make_shared<RequestVoteReply>();
    // 对方任期没有自己新
    if (vote_args->term() < current_term_) {
        vote_reply->set_term(current_term_);
        vote_reply->set_vote_granted(false);
        LOG_DEBUG << "rejected " << vote_args->candidate_id() << " 's vote request, args.term=" << vote_args->term() << " current_term_=" << current_term_;
    } else {   
        if (vote_args->term() > current_term_) {  // 任期大于自己的要初始化
            becomeFollower(vote_args->term());
        }
        vote_reply->set_term(current_term_);
        // (当前节点并没有投票 或者 已经投票过了且是对方节点) && 对方日志和自己一样新
        if ((vote_for_ == -1 || vote_for_ == vote_args->candidate_id()) 
            && !thisIsMoreUpToDate(vote_args->last_log_index(), vote_args->last_log_term())) {
            vote_for_ = vote_args->candidate_id();
            vote_reply->set_vote_granted(true);
            LOG_DEBUG << "vote for " << vote_args->candidate_id();
        } else {
            vote_reply->set_vote_granted(false);
            LOG_DEBUG << "rejected " << vote_args->candidate_id() << " 's vote request, vote_for=" << vote_for_;
        }
    }
    return vote_reply;
}

// 日志比较的方法：
// 1.最后一条日志的任期号。如果大说明新。如果小，说明不新。如果相等跳到2
// 2.判断索引长度。大的更新
bool Raft::thisIsMoreUpToDate(uint32_t last_log_index, uint32_t last_log_term) const {
	uint32_t this_last_log_index = getLastEntryIndex();
	uint32_t this_last_log_term = log_[this_last_log_index].term();
	return (this_last_log_term > last_log_term 
			|| (this_last_log_term == last_log_term && this_last_log_index > last_log_index));
}

void Raft::sendRequestVote()
{
    MutexGuard guard(mutex_);
    if (!running_)
        return;

    becomeCandidate();
    std::shared_ptr<RequestVoteArgs> args = std::make_shared<RequestVoteArgs>();
    args->set_term(current_term_);
    args->set_candidate_id(id_);
    args->set_last_log_index(1);
    args->set_last_log_term(0);
    LOG_DEBUG << "sendRequestVote, current_term_=" << current_term_;
    for (auto &peer : peers_) {
        peer->Call<RequestVoteReply>(args, std::bind(&Raft::onRequestVoteReply, this, std::placeholders::_1));
    }

    scheduler_->cancel(timeout_id_);
    timeout_id_ = scheduler_->runAfter(getElectionTimeout(), 
                                       std::make_shared<Coroutine>(std::bind(&Raft::sendRequestVote, this)));
    LOG_DEBUG << "timeout_id_=" << timeout_id_;
}

void Raft::onRequestVoteReply(std::shared_ptr<RequestVoteReply> reply)
{
    MutexGuard guard(mutex_);
    if (!running_ || state_ != Candidate)
        return;
    
    if (reply->term() > current_term_){
        becomeFollower(reply->term());
    } else if (reply->term() == current_term_ && reply->vote_granted()) {
        votes_++;
        if (votes_ > (peers_.size() + 1) / 2) {
            becomeLeader();
        }
    }
}

/**
 * 附加日志(多个日志,为了提高效率) RPC
 *
 * 接收者实现：
 *    如果 term < currentTerm 就返回 false （5.1 节）
 *    如果日志在 prevLogIndex 位置处的日志条目的任期号和 prevLogTerm 不匹配，则返回 false （5.3 节）
 *    如果已经存在的日志条目和新的产生冲突（索引值相同但是任期号不同），删除这一条和之后所有的 （5.3 节）
 *    附加任何在已有的日志中不存在的条目
 *    如果 leaderCommit > commitIndex，令 commitIndex 等于 leaderCommit 和 新日志条目索引值中较小的一个
 */
MessagePtr Raft::onRequestAppendEntry(std::shared_ptr<RequestAppendArgs> args)
{
    LOG_DEBUG << "onRequestAppendEntry";
    MutexGuard guard(mutex_);
    std::shared_ptr<RequestAppendReply> reply = std::make_shared<RequestAppendReply>();

    if (args->term() < current_term_) {
        reply->set_success(false);
        reply->set_term(current_term_);
        reply->set_conflict_index(id_);
        reply->set_conflict_term(current_term_);
    }
    else
    {
        becomeFollower(args->term());
        reply->set_success(true);
        reply->set_term(current_term_);
        reply->set_conflict_index(0);
        reply->set_conflict_term(0);
    }

    return reply;
}




void Raft::heartbeat()
{
    MutexGuard guard(mutex_);
    if (!running_ || state_ != Leader)
        return;
    std::shared_ptr<RequestAppendArgs> args = std::make_shared<RequestAppendArgs>();
    args->set_term(current_term_);
    args->set_leader_id(id_);
    args->set_pre_log_index(1);
    args->set_pre_log_term(1);
    args->set_leader_commit(1);
    for (auto &peer : peers_)
    {
        scheduler_->addTask([this, peer, args]() {
            LOG_DEBUG << "heartbeat";
            peer->Call<RequestAppendReply>(args, [](std::shared_ptr<RequestAppendReply> reply) {
                LOG_DEBUG << "reply->term=" << reply->term() << ", reply->success=" << reply->success();
                reply->set_term(0);
            });
        });
    }
}

void Raft::becomeFollower(uint32_t term)
{
    std::cout << "======================become Follower" << std::endl;
    if (state_ == Leader)
        scheduler_->cancel(heartbeat_id_);
    else
        scheduler_->cancel(timeout_id_);

    state_ = Follower;
    current_term_ = term;
    vote_for_ = -1;
    votes_ = 0;
    timeout_id_ = scheduler_->runAfter(getElectionTimeout(), 
                                       std::make_shared<Coroutine>(std::bind(&Raft::sendRequestVote, this)));
    LOG_DEBUG << "timeout_id_=" << timeout_id_;
}

void Raft::becomeCandidate()
{
    std::cout << "=====================become Candidate" << std::endl;
    ++current_term_;
    state_ = Candidate;
    vote_for_ = id_;
    votes_ = 1;
}

void Raft::becomeLeader()
{
    std::cout << "=====================become Leader" << std::endl;
    state_ = Leader;
    scheduler_->cancel(timeout_id_);
    resetLeaderState();
    // heartbeat();
    heartbeat_id_ = scheduler_->runEvery(kHeartbeatInterval, 
                                         std::make_shared<Coroutine>(std::bind(&Raft::heartbeat, this)));
}

void Raft::resetLeaderState() {
	next_index_.clear();
	match_index_.clear();
	next_index_.insert(next_index_.begin(), peers_.size(), log_.size());
	match_index_.insert(match_index_.begin(), peers_.size(), 0);
}

uint64_t Raft::getElectionTimeout()
{
    static std::default_random_engine engine(time(0));
    static std::uniform_int_distribution<uint64_t> dist(kTimeoutBase, kTimeoutTop);
    return dist(engine);
}