#ifndef _LEO_MUTEX_H_
#define _LEO_MUTEX_H_

#include <pthread.h>

#include "Noncopyable.h"

namespace leo {
//todo: uncopybale

class Mutex : public Noncopyable {
friend class Condition;
public:
	Mutex();
	~Mutex();

	void lock();
	void unlock();
private:
	pthread_mutex_t* getMutex();

	pthread_mutex_t mutex_;
};

class MutexGuard {
public:
	MutexGuard(Mutex& mutex);
	~MutexGuard();
private:
	Mutex& mutex_;
};

}
#endif

