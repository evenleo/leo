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


class TimerManager {
friend class Scheduler;
public:
	TimerManager();

	~TimerManager();

	/**
	 * 添加定时器
	 * @param when：定时时间
	 * @param coroutine: 协程函数
	 * @param processer: 所属处理器
	 * @param interval: 定时器间隔
	 */ 
	int64_t addTimer(uint64_t when, Coroutine::ptr coroutine, Processer* processer, uint64_t interval = 0);
	
	// 根据timer_id取消定时器
	void cancel(int64_t);

private:
	// 处理定时器队列的主要函数
	void dealExpiredTimer();

	bool findFirstTime(const uint64_t&, uint64_t&);

	ssize_t readTimerFd();

	void resetTimerFd(uint64_t when);
	
private:
	int timer_fd_;
	Mutex mutex_;

	std::multimap<uint64_t, Timer::ptr> timer_map_;  // timer是有序的
	// for cancel
	std::map<int64_t, uint64_t>   sequence_2_time_;  // 序列号对应的时间 
	std::set<int64_t> 			  cancel_set_;       // 取消的定时器集合
};

}

#endif
