#include "Log.h"
#include "Singleton.h"

#include <iostream>
#include <unistd.h>

using namespace std;
using namespace leo;

int main(int , char* argv[]) {
	//leo::Logger::setLogLevel(leo::LogLevel::INFO);
	Singleton<Logger>::getInstance()->addAppender("console", LogAppender::ptr(new ConsoleAppender()));
	std::shared_ptr<AsyncFileAppender> file_appender = std::make_shared<AsyncFileAppender>(argv[0]);	
	file_appender->start();
	Singleton<Logger>::getInstance()->addAppender("file", file_appender);

	for (int i = 0; i < 100; i++) {
		LOG_DEBUG << "debug";
		LOG_INFO << "info";
		LOG_WARN << "warn";
		LOG_ERROR << "error";
	}
	file_appender->stop();
	
	return 0;
}
