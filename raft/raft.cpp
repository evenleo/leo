#include "raft.h"
#include <functional>
#include <thread>


Raft::Raft(uint32_t me, int port,  Scheduler::ptr scheduler, std::vector<RpcClient::ptr>& peers) 
    : me_(me),
      state_(Follower),
      current_term_(0),
      vote_for_(-1),
      running_(false), 
      scheduler_(scheduler),
      peers_(std::move(peers))
{
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
    // 如果term < currentTerm返回 false （5.2 节）
    if (vote_args->term() < current_term_) {
        vote_reply->set_term(current_term_);
        vote_reply->set_vote_granted(false);
        LOG_DEBUG << "rejected " << vote_args->candidate_id() << " 's vote request, args.term=" << vote_args->term() << " current_term_=" << current_term_;
    } else { 
        // 如果接收到的 RPC 请求或响应中，任期号 > currentTerm，
        // 那么就令 currentTerm 等于 T，并切换状态为跟随者（5.1 节）  
        if (vote_args->term() > current_term_) {  
            becomeFollower(vote_args->term());
        }
        vote_reply->set_term(current_term_);
        // (当前节点并没有投票 或者 已经投票过了且是对方节点) && 对方日志和自己一样新
        if ((vote_for_ == -1 || vote_for_ == vote_args->candidate_id()) 
            && upToDate(vote_args->last_log_index(), vote_args->last_log_term())) {
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

 /**
 * Raft 通过比较两份日志中最后一条日志条目的索引值和任期号定义谁的日志比较新。
 * 比较逻辑：如果两份日志最后的条目的任期号不同，那么任期号大的日志更加新。
 * ********如果两份日志最后的条目任期号相同，那么日志比较长的那个就更加新。
 *
 * @return ret 候选人的日志至少和自己一样新,则为true；否则为false
 */
bool Raft::upToDate(uint32_t last_log_index, uint32_t last_log_term) const {
	uint32_t this_last_log_index = getLastEntryIndex();
	uint32_t this_last_log_term = log_[this_last_log_index].term();
	return (last_log_term > this_last_log_term 
			|| (this_last_log_term == last_log_term && last_log_index >= this_last_log_index));
}

void Raft::sendRequestVote()
{
    MutexGuard guard(mutex_);
    if (!running_)
        return;

    becomeCandidate();
    std::shared_ptr<RequestVoteArgs> args = std::make_shared<RequestVoteArgs>();
    args->set_term(current_term_);
    args->set_candidate_id(me_);
    args->set_last_log_index(1);
    args->set_last_log_term(0);
    LOG_DEBUG << "sendRequestVote, current_term_=" << current_term_;
    for (uint32_t i = 0; i < peers_.size(); ++i) {
        if (i != me_) {
            peers_[i]->Call<RequestVoteReply>(args, std::bind(&Raft::onRequestVoteReply, this, std::placeholders::_1));
        }
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
    // LOG_DEBUG << "onRequestAppendEntry";
    MutexGuard guard(mutex_);
    std::shared_ptr<RequestAppendReply> reply = std::make_shared<RequestAppendReply>();

    reply->set_term(current_term_);
    if (args->term() < current_term_) {
        reply->set_success(false);
    } else {
        becomeFollower(args->term());

        uint32_t prev_log_index = args->prev_log_index();
        if (prev_log_index == 0 || (prev_log_index <= getLastEntryIndex() 
                                    && log_[prev_log_index].term() == args->prev_log_term())) {
            reply->set_success(true);
            // LOG_DEBUG << "get " << args->entries_size() << " entry from leader=" << args->leader_id();
            if (args->entries_size() > 0) {
                log_.erase(log_.begin() + prev_log_index + 1, log_.end());

                std::vector<LogEntry> newLogs;
                for (int i = 0; i < args->entries_size(); ++i) {
                    const LogEntry& entry = args->entries(i);
                    newLogs.push_back(entry);
                }
                // cherry 加上这里，应该是不加的才对
                // const LogEntry& lastEntry = args->entries(args->entries_size() - 1);
                // uint32_t index = prev_log_index + args->entries_size() + 1;
                // if (index <= getLastEntryIndex() && log_[index].term() >= lastEntry.term()) {
                //     for (uint32_t i = index; i < log_.size(); ++i) {
                //         newLogs.push_back(log_[i]);
                //     }
                // }

                log_.insert(log_.end(), newLogs.begin(), newLogs.end());
            }

            if (args->leader_commit() > commit_index_) {
                commit_index_ = std::min(args->leader_commit(), getLastEntryIndex());
            }
        } else {
            reply->set_success(false);
        }
    }
    
    return reply;
}

 void Raft::onRequestAppendReply(uint32_t peer_id, 
                                 std::shared_ptr<RequestAppendArgs> append_args, 
                                 std::shared_ptr<RequestAppendReply> reply) 
{
    MutexGuard guard(mutex_);
    if (!running_ || state_ != Leader || current_term_ != append_args->term()) {
        return;
    }

    if (reply->success()) {
        // 成功后更新leader的next_index和match_index，并且更新更新commit_index
        match_index_[peer_id] = append_args->prev_log_index() + append_args->entries_size();
        next_index_[peer_id] = match_index_[peer_id] + 1;
        // toDo 更新commit_index
    } else {
        // 返回false有两种原因:1.出现term比当前server大的， 
        // 2.参数中的PreLogIndex对应的Term和目标server中index对应的Term不一致, next_index_[peer_id]减一
        if (append_args->term() < reply->term()) {
            becomeFollower(reply->term());
        } else if (next_index_[peer_id] > 1) {  // log_是从1开始同步
            next_index_[peer_id]--;
        }
    }
}

void Raft::heartbeat()
{
    MutexGuard guard(mutex_);
    if (!running_ || state_ != Leader)
        return;
   
    for (uint32_t i = 0; i < peers_.size(); ++i)
    {
        if (i == me_) continue;
        std::shared_ptr<RequestAppendArgs> args = std::make_shared<RequestAppendArgs>();
        args->set_term(current_term_);
        args->set_leader_id(me_);
        int next_index = next_index_[i];
        args->set_prev_log_index(next_index - 1);
        args->set_prev_log_term(log_[next_index - 1].term());
        args->set_leader_commit(commit_index_);
        packEntrys(next_index, args);
        peers_[i]->Call<RequestAppendReply>(args, std::bind(&Raft::onRequestAppendReply, this, i, args, std::placeholders::_1));
    }
}

void Raft::packEntrys(size_t next_index, std::shared_ptr<RequestAppendArgs> append_args) {
	for (size_t i = next_index; i < log_.size(); ++i) {
		const LogEntry& originEntry = log_[i];
		LogEntry* entry = append_args->add_entries();
		entry->set_term(originEntry.term());
		entry->set_index(originEntry.index());
		entry->set_command(originEntry.command());
	}
}

void Raft::becomeFollower(uint32_t term)
{
    if (state_ != Follower)
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
    // LOG_DEBUG << "timeout_id_=" << timeout_id_;
}

void Raft::becomeCandidate()
{
    std::cout << "=====================become Candidate" << std::endl;
    ++current_term_;
    state_ = Candidate;
    vote_for_ = me_;
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