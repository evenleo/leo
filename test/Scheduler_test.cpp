#include "Scheduler.h"
#include "Log.h"
#include <assert.h>

using namespace leo;

Scheduler* g_scheduler;

void foo() {
	std::cout << "in foo 00000000000000000000000000000" << std::endl;
	leo::Processer* processer = leo::Processer::GetProcesserOfThisThread();
	leo::Scheduler* scheduler = processer->getScheduler();
	assert(scheduler != nullptr);
	scheduler->runAt(leo::Timer::getCurrentMs() + 1 * leo::kMicrosecondsPerSecond, leo::Coroutine::GetCurrentCoroutine());
	leo::Coroutine::SwapOut();
	std::cout << "leave foo 00000000000000000000000000000" << std::endl;
}

int main() {
	Singleton<Logger>::getInstance()->addAppender("console", LogAppender::ptr(new ConsoleAppender()));
	Scheduler scheduler;
	g_scheduler = &scheduler;
	scheduler.startAsync();

	scheduler.addTask(foo, "foo");
	
	scheduler.wait();
	return 0;
}

