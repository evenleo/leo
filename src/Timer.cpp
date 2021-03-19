#include <iostream>
#include "Timer.h"

namespace leo {

Timer::Timer(uint64_t time, Processer* processer, Coroutine::ptr coroutine, uint64_t interval)
  : time_(time),
	processer_(processer),
	coroutine_(coroutine),
	interval_(interval),
	sequence_(s_sequence_creator_++) 
{}

uint64_t Timer::getCurrentMs() {
	struct timeval tv;
	if (gettimeofday(&tv, nullptr)) {
		std::cerr << "gettimeofday:" << strerror(errno) << std::endl;
	}
	return tv.tv_sec * kMicrosecondsPerSecond + tv.tv_usec;
}
	
std::atomic<int64_t> Timer::s_sequence_creator_;

}
