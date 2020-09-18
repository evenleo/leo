#ifndef _LEO_THREAD_H_
#define _LEO_THREAD_H_

#include "Noncopyable.h"
#include <functional>

namespace leo {

class Thread : public Noncopyable {
public:
	typedef std::function<void ()> Func;
	Thread(Func cb, std::string name = "");
	~Thread();

	bool isStarted() const { return started_; }
	void start();
	void join();
	const std::string& getName() const { return name_; }
	
	static pid_t CurrentThreadTid();
private:
	static void* threadFuncInternal(void* arg);
	bool started_;
	bool joined_;
	pthread_t tid_;
	std::string name_;
	Func cb_;
};

}

#endif 
