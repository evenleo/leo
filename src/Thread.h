#ifndef _LEO_THREAD_H_
#define _LEO_THREAD_H_

#include "Noncopyable.h"
#include <functional>

namespace leo {

class Thread : public Noncopyable {
public:
	typedef std::function<void ()> Func;

	Thread(Func cb, const std::string& name = "");

	~Thread();

	void start();

	void join();

	bool isStarted() const { return started_; }

	const std::string& getName() const { return name_; }

	static pid_t CurrentThreadTid();

private:
	static void* threadFuncInternal(void* arg);

	bool started_;       // 开始
	bool joined_;        // join标志
	pthread_t tid_;      // 线程id
	std::string name_;   // 线程名称
	Func cb_;            // 回调函数
};

}

#endif 
