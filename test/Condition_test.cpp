#include "Condition.h"
#include "Log.h"
#include "Thread.h"

#include <memory>
#include <unistd.h>
#include <vector>
#include "Mutex.h"

using namespace leo;

int count = 0;

void notify_test() {
	Mutex mutex;
	Condition cond(mutex);

	std::vector<std::shared_ptr<Thread>> v;

	for (int i = 0; i < 10; ++i) {
		v.push_back(std::make_shared<Thread>([&]() {
									MutexGuard guard(mutex);
									cond.wait();
									LOG_DEBUG << "+1";
								}));
	}

	for (auto& i : v) {
		i->start();
	}

	cond.notify();
	LOG_DEBUG << "----------";
	sleep(2);
	LOG_DEBUG << "==========";
}

void notifyAll_test() {
	Mutex mutex;
	Condition cond(mutex);

	std::vector<std::shared_ptr<Thread>> v;

	for (int i = 0; i < 10; ++i) {
		v.push_back(std::make_shared<Thread>([&]() {
									MutexGuard guard(mutex);
									cond.wait();
									count++;
									LOG_DEBUG << "+1";
								}));
	}

	for (auto& i : v) {
		i->start();
	}

	cond.notifyAll();
	LOG_DEBUG << "----------";
	sleep(2);
	LOG_DEBUG << "==========";
	LOG_DEBUG << "count=" << count;
}

void wait_seconds_test() {
	Mutex mutex;
	Condition cond(mutex);

	std::vector<std::shared_ptr<Thread>> v;

	for (int i = 0; i < 1; ++i) {
		v.push_back(std::make_shared<Thread>([&]() {
									MutexGuard guard(mutex);
									cond.wait_seconds(2);
									LOG_DEBUG << "wait up";
								}));
	}

	for (auto& i : v) {
		i->start();
	}

	LOG_DEBUG << "----------";
	sleep(5);
	LOG_DEBUG << "==========";
}

int main() {
	Singleton<Logger>::getInstance()->addAppender("console", LogAppender::ptr(new ConsoleAppender()));
	LOG_DEBUG << "test notify ..........";
	notify_test();
	LOG_DEBUG << "test notifyAll ..........";
	notifyAll_test();
	LOG_DEBUG << "test wait_seconds ..........";
	wait_seconds_test();
	return 0;
}
