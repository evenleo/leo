#include "Scheduler.h"
#include "Log.h"

#include <unistd.h>

using namespace leo;

int main() {
	Singleton<Logger>::getInstance()->addAppender("console", LogAppender::ptr(new ConsoleAppender()));

	auto scheduler = leo::Singleton<Scheduler>::getInstance();
	scheduler->addTask([](){
						LOG_DEBUG << "before sleep";
						sleep(5);
						LOG_DEBUG << "after sleep";
					});
	scheduler->start();
	return 0;
}
