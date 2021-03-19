#ifndef _LEO_COROUTINE_SCHEDULER_H_
#define _LEO_COROUTINE_SCHEDULER_H_

#include "Coroutine.h"
#include "Noncopyable.h"
#include "Poller.h"
#include "Mutex.h"

#include <list>

namespace leo {

class Scheduler;
class Processer : public Noncopyable {
public:
	typedef std::shared_ptr<Processer> ptr;

	Processer(Scheduler* scheduler);
	virtual ~Processer() {}

	virtual void run();

	void stop();

	bool stoped() { return stop_; }

	size_t getLoad() { return load_; }

	Scheduler* getScheduler() { return scheduler_; }

	void addTask(Coroutine::ptr coroutine);

	void addTask(Coroutine::Func func, std::string name = "");

	void updateEvent(int fd, int events, Coroutine::ptr coroutine = nullptr);
	
	void removeEvent(int fd);

	static Processer*& GetProcesserOfThisThread();  // 指针的引用，是左值，可以赋值

private:
	// 唤醒poll协程
	void wakeupPollCoroutine();
	// 消费唤醒事件
	ssize_t comsumeWakeEvent();

	bool stop_ = false;
	size_t load_ = 0;
	Mutex mutex_;
	Scheduler* scheduler_;
	EventPoller poller_;
	int event_fd_;
	std::list<Coroutine::ptr> coroutines_;
};

}

#endif
