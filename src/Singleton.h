#ifndef _LEO_SINGLETON_H_
#define _LEO_SINGLETON_H_

#include "Noncopyable.h"

#include <pthread.h>
#include <iostream>

namespace leo {

template <typename T>
class Singleton {
public:
	static T* getInstance() {
		static T instance;
		return &instance;
	}
};

}

#endif
