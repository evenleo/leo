#include "Log.h"

#include <assert.h>
#include <atomic>
#include <sys/syscall.h>
#include <string.h>
#include <Thread.h>
#include <unistd.h>

namespace leo {

std::atomic<int> threadCount(0);
static __thread pid_t t_tid = 0;

pid_t Thread::CurrentThreadTid() {
	if (t_tid == 0) {
		t_tid = ::syscall(SYS_gettid);
	}
	return t_tid;
}

Thread::Thread(Func cb, std::string name)
	: started_(false), joined_(false), cb_(std::move(cb)) 
{
	if (name.empty()) {
		int num = threadCount.fetch_add(1);
		name_ = "Thread-" + std::to_string(num);
	} else {
		name_ = name;
	}
}

Thread::~Thread() 
{
	if (started_ && !joined_) {
		pthread_detach(tid_);
	}
}

void Thread::start() 
{
	assert(!started_);
	started_ = true;
	if (int error = pthread_create(&tid_, nullptr, Thread::threadFuncInternal, this)) {
		started_ = false;	
		LOG_FATAL << "pthread_create failed, " << strerror(error);	
	}
}

void Thread::join() 
{
	assert(started_);
	assert(!joined_);
	joined_ = true;
	if (int error = pthread_join(tid_, nullptr)) {
		joined_ = false;
		LOG_FATAL << "pthread_join failed, " << strerror(error);	
	}
}

void* Thread::threadFuncInternal(void* arg) 
{
	Thread* thread = static_cast<Thread*>(arg);	
	Func cb;
	cb.swap(thread->cb_);
	cb();
	return 0;
}

}
