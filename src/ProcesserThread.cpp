#include "Processer.h"
#include "Log.h"
#include "ProcesserThread.h"

#include <assert.h>

namespace leo {

ProcessThread::ProcessThread(Scheduler* scheduler) 
	:thread_(std::bind(&ProcessThread::threadFunc, this)),
	scheduler_(scheduler),
	processer_(nullptr),
	mutex_(),
	cond_(mutex_) {

}

Processer* ProcessThread::startProcess() {
	thread_.start();

	MutexGuard lock(mutex_);
	while (processer_ == nullptr) {
		cond_.wait();
	}
	assert(processer_ != nullptr);
	return processer_;
}

void ProcessThread::join() {
	thread_.join();
}

void ProcessThread::threadFunc() {
	Processer processer(scheduler_);  // 局部变量不用担心生命周期

	{
		MutexGuard guard(mutex_);
		processer_ = &processer;
		cond_.notify();
	}

	processer.run();
}

}
