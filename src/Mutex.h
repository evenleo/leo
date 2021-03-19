#ifndef _LEO_MUTEX_H_
#define _LEO_MUTEX_H_

#include <pthread.h>

#include "Noncopyable.h"

namespace leo {

class Mutex : public Noncopyable {
friend class Condition;
public:
	Mutex() { pthread_mutex_init(&mutex_, nullptr); }
	~Mutex() { pthread_mutex_destroy(&mutex_); }

	void lock() { pthread_mutex_lock(&mutex_); }
	void unlock() { pthread_mutex_unlock(&mutex_); }
	
private:
	pthread_mutex_t* getMutex() { return &mutex_; }
	pthread_mutex_t mutex_;
};

class MutexGuard {
public:
	MutexGuard(Mutex& mutex) : mutex_(mutex) { mutex_.lock(); }
	~MutexGuard() { mutex_.unlock(); }

private:
	Mutex& mutex_;
};

}

#endif

