#ifndef _LEO_SCHEDULER_H_
#define _LEO_SCHEDULER_H_

#include "Coroutine.h"
#include "Noncopyable.h"
#include "ProcesserThread.h"
#include "Processer.h"
#include "Singleton.h"
#include "Timestamp.h"

#include <vector>
#include "Mutex.h"

namespace leo {
	
class TimerManager;

class Scheduler : public Noncopyable {
friend class Singleton<Scheduler>;
public:
	typedef std::shared_ptr<Scheduler> ptr;

	Scheduler(size_t thread_number = 1);
	~Scheduler();

	void start();
	void startAsync();
	void wait();
	void stop();
	void addTask(Coroutine::Func task, std::string name = "");
	int64_t runAt(Timestamp when, Coroutine::ptr coroutine);
	int64_t runAfter(uint64_t micro_delay, Coroutine::ptr coroutine);
	int64_t runEvery(uint64_t micro_interval, Coroutine::ptr coroutine);
	void cancel(int64_t);

protected:
	Processer* pickOneProcesser();
private:
	void joinThread();
private:
	bool running_ = false;
	size_t thread_num_;
	Processer main_processer_;
	std::vector<Processer*> work_processers_;
	std::vector<ProcessThread::ptr> threads_;

	//单独一个线程处理定时任务
	Processer* timer_processer_;
	ProcessThread::ptr timer_thread_;
	std::unique_ptr<TimerManager> timer_manager_;

	Thread thread_;

	Mutex mutex_;
	Condition cond_;
	Condition quit_cond_;

	//for stop
	Thread join_thread_;
};

}

#endif
