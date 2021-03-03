#include "Scheduler.h"
#include "Log.h"

#include <unistd.h>

using namespace leo;

int main() {
	Singleton<Logger>::instance()->addAppender("console", LogAppender::ptr(new ConsoleAppender()));

	auto scheduler = leo::Singleton<Scheduler>::instance();
	scheduler->addTask([](){
						LOG_DEBUG << "before sleep";
						sleep(5);
						LOG_DEBUG << "after sleep";
					});
	scheduler->startAsync();
	sleep(10);
	return 0;
}
