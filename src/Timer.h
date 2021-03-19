#ifndef _LEO_TIMER_H_
#define _LEO_TIMER_H_

#include <stdint.h>
#include <string>
#include <sys/time.h>
#include <errno.h>
#include <string.h>
#include <atomic>
#include "Coroutine.h"

namespace leo {

const static uint64_t kMicrosecondsPerSecond = 1000 * 1000;

class Processer;
class Timer {
public:
	typedef std::shared_ptr<Timer> ptr;
	Timer(uint64_t time, Processer* processer, Coroutine::ptr coroutine, uint64_t interval);
	
	/**
	 * 获取当期时间
	 */
	static uint64_t getCurrentMs();

	void setTime(uint64_t time) { time_ = time; }

	uint64_t getTime() const { return time_; }

	void setCoroutine(Coroutine::ptr coroutine) { coroutine_ = coroutine; }

	Coroutine::ptr getCoroutine() const { return coroutine_; }

	Processer* getProcesser() const { return processer_; }

	uint64_t getInterval() const { return interval_; }

	int64_t  getSequence() const { return sequence_; }

	
private:
	uint64_t time_;                // 时间
	Processer* processer_;         // 所属处理器
	Coroutine::ptr coroutine_;     // 所属协程
	uint64_t interval_;            // 间隔
	int64_t sequence_;             // timer有序列号
	static std::atomic<int64_t> s_sequence_creator_;  // timer的序列号生成器，递增
};

}

#endif
