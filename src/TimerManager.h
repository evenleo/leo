#ifndef _LEO_TIMER_MANAGER_H_
#define _LEO_TIMER_MANAGER_H_

#include "Coroutine.h"
#include "Timestamp.h"

#include <atomic>
#include <map>
#include <memory>
#include <sys/timerfd.h>
#include <string.h>
#include <set>
#include <unistd.h>
#include "Mutex.h"

namespace leo {

class Processer;
class Timer {
public:
	typedef std::shared_ptr<Timer> Ptr;
	Timer(Timestamp timestamp, Processer* processer, Coroutine::ptr coroutine, uint64_t interval)
   		: timestamp_(timestamp),
		  processer_(processer),
		  coroutine_(coroutine),
		  interval_(interval),
		  sequence_(s_sequence_creator_++) {}

	void setTimestamp(Timestamp timestamp) { timestamp_ = timestamp; }
	Timestamp getTimestamp() const { return timestamp_; }
	Processer* getProcesser() const { return processer_; }
	Coroutine::ptr getCoroutine() const { return coroutine_; }
	void setCoroutine(Coroutine::ptr coroutine) { coroutine_ = coroutine; }
	uint64_t getInterval() const { return interval_; };
	int64_t  getSequence() const { return sequence_; };
private:
	Timestamp timestamp_;
	Processer* processer_;
	Coroutine::ptr coroutine_;
	uint64_t interval_;
	int64_t sequence_;
	static std::atomic<int64_t> s_sequence_creator_;
};

class Processer;

int createTimerFd();

class TimerManager {
friend class Scheduler;
public:
	typedef std::function<void ()> Callback;
	TimerManager() 
		:timer_fd_(createTimerFd()) {}
	~TimerManager() {
		::close(timer_fd_);
	}
	
	int64_t addTimer(Timestamp when, Coroutine::ptr coroutine, Processer* processer, uint64_t interval = 0);
	void cancel(int64_t);
private:
	bool findFirstTimestamp(const Timestamp&, Timestamp&);
	ssize_t readTimerFd();
	void resetTimerFd(Timestamp when);
	void dealWithExpiredTimer();

	int timer_fd_;
	std::multimap<Timestamp, Timer::Ptr> timer_map_;
	Mutex mutex_;

	// for cancel
	std::map<int64_t, Timestamp>  sequence_2_timestamp_;
	std::set<int64_t> 			  cancel_set_;
};

}

#endif
