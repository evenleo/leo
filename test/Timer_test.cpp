#include "Scheduler.h"

#include <unistd.h>

using namespace leo;

int main() {
	Scheduler scheduler;
	scheduler.startAsync();

	/**
	 *  当timer_fd没有cancel之前，会每隔2s执行以下timeout回调，休眠10scancel，测试执行4次回调
	 *  当第一个参数为0时，只调用一次回调
	 */ 
	int64_t timer_id = scheduler.runEvery(2 * kMicrosecondsPerSecond, std::make_shared<Coroutine>([](){
									printf("timeout\n");
								}));
	sleep(10);  
	scheduler.cancel(timer_id);
	printf("cancel\n");

	return 0;
}
