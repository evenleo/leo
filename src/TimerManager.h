#ifndef _LEO_TIMER_MANAGER_H_
#define _LEO_TIMER_MANAGER_H_

#include "Coroutine.h"
#include "Timer.h"

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

int createTimerFd();

class TimerManager {
friend class Scheduler;
public:
	typedef std::function<void ()> Callback;

	TimerManager() : timer_fd_(createTimerFd()) {}

	~TimerManager() { ::close(timer_fd_); }
	
	int64_t addTimer(uint64_t when, Coroutine::ptr coroutine, Processer* processer, uint64_t interval = 0);

	void cancel(int64_t);

private:
	bool findFirstTime(const uint64_t&, uint64_t&);

	ssize_t readTimerFd();

	void resetTimerFd(uint64_t when);
	
	void dealExpiredTimer();

	int timer_fd_;
	Mutex mutex_;
	std::multimap<uint64_t, Timer::ptr> timer_map_;  // timer是有序的

	// for cancel
	std::map<int64_t, uint64_t>  sequence_2_time_;
	std::set<int64_t> 			  cancel_set_;
};

}

#endif
