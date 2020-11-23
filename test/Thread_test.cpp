#include "Thread.h"
#include "Log.h"
#include <sys/syscall.h>
#include <unistd.h>

using namespace std;
using namespace leo;

void func() {
	LOG_DEBUG << "thread func: " << Thread::CurrentThreadTid();
	while (1) {
		LOG_DEBUG << "poll thread id=" << Thread::CurrentThreadTid();
		sleep(1);
	}
}

int main() {
	Singleton<Logger>::getInstance()->addAppender("console", LogAppender::ptr(new ConsoleAppender()));

	Thread t1(func, "leo_thread");
	Thread t2(func);

	LOG_DEBUG << "t1 name=" << t1.getName();
	LOG_DEBUG << "t2 name=" << t2.getName();
	LOG_DEBUG << "main thread: " << Thread::CurrentThreadTid();
	LOG_DEBUG << "thread start ...";

	t1.start();
	t2.start();

	t1.join();	
	t2.join();
	
	return 0;
}
