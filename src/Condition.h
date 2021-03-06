#ifndef _LEO_CONDITION_H_
#define _LEO_CONDITION_H_

#include "Mutex.h"
#include "Noncopyable.h"

#include <pthread.h>

namespace leo {

class Condition : public Noncopyable {
public:
	explicit Condition(Mutex& mutex);

	~Condition();

	void wait();

	bool wait_seconds(time_t seconds);

	void notify();
	
	void notifyAll();
	
private:
	Mutex& mutex_;
	pthread_cond_t cond_;
};

}

#endif
