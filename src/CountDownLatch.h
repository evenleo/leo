#ifndef _LEO_COUNT_DOWN_LATCH_H_
#define _LEO_COUNT_DOWN_LATCH_H_

#include "Noncopyable.h"
#include "Condition.h"

namespace leo {

class CountDownLatch : public Noncopyable {
public:
	CountDownLatch(int count);

	void wait();
	void countDown();
private:
	int count_;
	Mutex mutex_;
	Condition cond_;
};

}

#endif
