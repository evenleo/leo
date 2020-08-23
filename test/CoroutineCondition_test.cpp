#include "Scheduler.h"

#include <unistd.h>

using namespace leo;
using namespace std;

CoroutineCondition cond;

void bar() {
	cout << "bar(): start bar" << endl;
	cond.wait();
	cout << "bar(): pass wait" << endl;
}

void foo() {
	cout << "foo(): start sleep" << endl;
	sleep(3);
	cout << "foo(): notify" << endl;
	cond.notify();
}

int main() {
	Scheduler scheduler;
	scheduler.startAsync();

	scheduler.addTask(bar);
	scheduler.addTask(foo);

	return 0;
}
