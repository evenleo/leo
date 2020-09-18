#ifndef _LEO_SCHEDULER_THREAD_H_
#define _LEO_SCHEDULER_THREAD_H_

#include "Noncopyable.h"
#include "Thread.h"

#include <memory>
#include "Mutex.h"
#include "Condition.h"

namespace leo {

class Scheduler;
class Processer;

class ProcessThread : public Noncopyable {
public:
	typedef std::shared_ptr<ProcessThread> ptr;

	ProcessThread(Scheduler* scheduler);
	~ProcessThread() {}

	Processer* startProcess();
	void join();

private:
	void threadFunc();
	Thread thread_;	
	Scheduler* scheduler_;
	Processer* processer_;
	Mutex mutex_;
	Condition cond_;
};

}


#endif
