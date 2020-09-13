## 介绍

常见的服务器编程模型有如下两种：
1. 每个请求创建一个线程，使用阻塞式IO操作（或者叫thread per connection）。这种模型的优点是可以使用阻塞操作，缺点是伸缩性不强，每台机器能创建的线程是有限的，32位的机器应该不超过400个。
2. 非阻塞IO+IO多路复用（或者叫one loop per thread或者Reactor）+ 线程池。

leo是基于Reactor模式的Linux C++网络服务框架，集合了上述两种方式，实现了协程的概念，对一些函数进行了hook，所以可以像操作阻塞IO一样进行编程。

## 使用
在工程主目录下新建build目录，进入build目录，
```
cmake ..
make
```
编译完成后，test中的可执行程序分别位于build目录下。

以TCP服务端为例，
``` c++
void handleClient(TcpConnection::ptr conn){
	conn->setTcpNoDelay(true);
	Buffer::ptr buffer = std::make_shared<Buffer>();
	while (conn->read(buffer) > 0) {
		LOG_INFO << "recv: " << buffer->peekAsString();
		conn->write(buffer);
	}
	conn->close();
}

int main(int args, char* argv[]) {
	Singleton<Logger>::getInstance()->addAppender("console", LogAppender::ptr(new ConsoleAppender()));
	IpAddress listen_addr(5000);
	int threads_num = 3;
	Scheduler scheduler(threads_num);
	scheduler.startAsync();
	TcpServer server(listen_addr, &scheduler);
	server.setConnectionHandler(handleClient);
	server.start();

	scheduler.wait();
	return 0;
}
```
只需要为TcpServer设置连接处理函数，在连接处理函数中，参数TcpConnection::ptr conn代表此次连接，可以像阻塞IO一样进行读写，如果发生阻塞，当前协程会被切出去，直到可读或者可写事件到来时，该协程会被重新执行。

## 实现
### 日志库
#### 需求
1. 有多种日志级别，DEBUG, INFO, WARN, ERROR, FATAL。
2. 可以有多个目的地，比如文件，控制台，可以拓展。
3. 日志文件达到指定大小时自动roll。
4. 时间戳精确到微秒。使用gettimeofday(2)，在x86-64Linux上不会陷入内核。
5. 线程安全。
6. 写日志过程不能是同步的，否则会阻塞IO线程。

这是个典型的生产者-消费者问题。产生日志的线程将日志先存到缓冲区，日志消费线程将缓冲区中的日志写到磁盘。

#### 总体结构如下
![日志结构](https://blog-1253119293.cos.ap-beijing.myqcloud.com/other/melon_github_readme/%E6%97%A5%E5%BF%97_%E7%BB%93%E6%9E%84%E5%9B%BE.png)

每条LOG_DEBUG等语句对应创建一个匿名LogWrapper对象，同时搜集日志信息保存到LogEvent对象中，匿名对象创建完毕就会调用析构函数，在LogWrapper析构函数中将LogEvent送到Logger中，Logger再送往不同的目的地，比如控制台，文件等。

#### 异步文件Appender实现
AsyncFileAppend对外提供append方法，前端Logger只需要调用这个方法往里面塞日志，不用担心会被阻塞。

前端和后端都维护一个缓冲区。
第一种情况：前端写日志较慢，三秒内还没写满一个缓冲区。后端线程会被唤醒，进入临界区，在临界区内交换两个buffer的指针，出临界区后前端cur指向的缓冲区又是空的了，后端buffer指向的缓冲区为刚才搜集了日志的缓冲区，后端线程随后将buffer指向的缓冲区中的日志写到磁盘中。临界区内只交换两个指针，所以临界区很小。
![情况1](https://blog-1253119293.cos.ap-beijing.myqcloud.com/other/melon_github_readme/%E6%97%A5%E5%BF%97_%E6%83%85%E5%86%B51.png)


第二种情况：前端写日志较快，三秒内已经写满了一个缓冲区。比如两秒的时候已经写满了第一个缓冲区，那么将cur指针保存到一个向量buffers_中，然后开辟一块新的缓冲区，另cur指向这块新缓冲区。然后唤醒后端消费线程，后端线程进入临界区，将cur和后端buffer_指针进行交换，将前端buffers_向量和后端persist_buffers_向量进行swap(对于std::vector也是指针交换)。出了临界区后，前端的cur始终指向一块干净的缓冲区，前端的向量buffers_也始终为空，后端的persist_buffers_向量中始终保存着有日志的缓冲区的指针。临界区同样很小仅仅是几个指针交换。
![情况2](https://blog-1253119293.cos.ap-beijing.myqcloud.com/other/melon_github_readme/%E6%97%A5%E5%BF%97_%E6%83%85%E5%86%B52.png)

### 配置库
主要是实现读取基本配置信息的功能。配置信息如下
```
[reactor]
maxConn = 1024
threadNum = 5
ip = 127.0.0.1
port = 7779
```
读取配置文件代码
```c++
int main(int argc, char** argv)
{
    Singleton<Logger>::getInstance()->addAppender("console", LogAppender::ptr(new ConsoleAppender()));
    Singleton<Config>::getInstance()->setPath("../conf/config.conf");
    std::string ip;
    ip = Singleton<Config>::getInstance()->getString("reactor", "ip", ip);
    LOG_INFO << "ip=" << ip;
    return 0;
}
```

### 协程
#### 类图
![协程类图](https://blog-1253119293.cos.ap-beijing.myqcloud.com/other/melon_github_readme/%E5%8D%8F%E7%A8%8B_%E7%B1%BB%E5%9B%BE.png)

成员变量：
1. c_id_：当前协程id。
2. context_：协程上下文。
3. cb_：协程执行的函数。
4. stack_size_：协程栈大小。
5. statck_：协程栈。
6. state_：协程状态。

成员函数：
1. swapIn()：执行当前协程，只能由主协程调用。
2. SwapOut()：静态函数，让出当前协程的CPU，执行主协程，主协程会进行协程调度，将CPU控制权转到另一个协程。
3. GetCurrentCoroutine()：获取当前线程正在执行的协程。
4. GetMainCoroutine()：获取当前线程的的主协程。

#### 原理
ucontext系列函数：
1. `int getcontext(ucontext_t *ucp)`： 将此刻的上下文保存到ucp指向的结构中。
2. `int setcontext(const ucontext_t *ucp)`： 调用成功后不会返回，执行流转移到ucp指向的上下文。
3. `void makecontext(ucontext_t *ucp, void (*func)(), int argc, ...)`：重新设置ucp指向的上下文为func函数起始处。ucp结构由getcontext()获取。后续以ucp为参数调用setcontext()或者swapcontext()执行流将转到func函数。
4. `int swapcontext(ucontext_t *oucp, const ucontext_t *ucp)`：保存当前上下文到oucp，并激活ucp指向的上下文。

#### 需要考虑的问题
##### 协程栈大小
不能太大：协程多了，内存浪费。
不能太小：使用者可能无意在栈上分配一个缓冲区，导致栈溢出。
暂时先固定为128K。

##### 调度策略
目前是非抢占式调度。只能由协程主动或者协程执行完毕，才会让出CPU。

##### 协程同步
两个协程间可能需要同步操作，比如协程1需要等待某个条件才能继续运行，线程2修改条件然后通知协程1。
目前实现了简陋的wait/notify机制，见CoroutineCondition。

### 协程调度
#### 类图
![协程调度](https://blog-1253119293.cos.ap-beijing.myqcloud.com/other/melon_github_readme/%E5%8D%8F%E7%A8%8B%E8%B0%83%E5%BA%A6_%E7%B1%BB%E5%9B%BE.png)

##### Processer
线程栈上的对象，线程退出后自动销毁，生命周期大可不必操心。

成员变量：
1. poller_：Poller。
2. coroutines_：当前线程待执行的协程队列。

成员函数：
1. addTask()：添加任务。
2. run()：开始进行协程调度。

#### 协程调度示意图
![协程调度示意图](https://blog-1253119293.cos.ap-beijing.myqcloud.com/other/melon_github_readme/%E5%8D%8F%E7%A8%8B%E8%B0%83%E5%BA%A6_%E7%A4%BA%E6%84%8F%E5%9B%BE.png)

每个线程都有一个本地变量t_cur_cotourine指向当前正在执行的协程对象。

##### 调度过程
Processer.run()函数作为Main协程进行调度，没有协程在协程队列时，执行Poll协程，该协程执行poll()函数。以read操作为例，某个协程在执行read的操作时，如果数据没有准备好，就会将<fd， 当前协程对象>对注册到Poller中，然后挂起。如果所有协程都阻塞了，那么会执行Poll协程等待poll()函数返回，poll()函数返回后，如果有事件发生，会根据之前注册的<fd， 协程对象>，将协程对象重新加入调度队列，此时read已经有数据可读了。

Main协程对应的代码逻辑如下：
``` c++
void Processer::run() {
	if (GetProcesserOfThisThread() != nullptr) {
		LOG_FATAL << "run two processer in one thread";
	} else {
		GetProcesserOfThisThread() = this;
	}
	leo::setHookEnabled(true);
	Coroutine::ptr cur;

	//没有可以执行协程时调用poll协程
	Coroutine::ptr poll_coroutine = std::make_shared<Coroutine>(std::bind(&Poller::poll, &poller_, kPollTimeMs), "Poll");

	while (!stop_) {
		{
			MutexGuard guard(mutex_);
			//没有协程时执行poll协程
			if (coroutines_.empty()) {
				cur = poll_coroutine;
				poller_.setPolling(true);
			} else {
				for (auto it = coroutines_.begin();
						it != coroutines_.end();
							++it) {
					cur = *it;
					coroutines_.erase(it);
					break;
				}
			}
		}
		cur->swapIn();
		if (cur->getState() == CoroutineState::TERMINATED) {
			load_--;
		}
	}
}
```

Poll协程对应的代码逻辑如下：
``` c++
void EventPoller::poll(int timeout) {
	const uint64_t MAX_EVENTS = 1024;
	epoll_event events[MAX_EVENTS];
	while (!processer_->stoped()) {
		is_polling_ = true;
		int nfds = epoll_wait(epfd_, events, MAX_EVENTS, timeout);
		is_polling_ = false;
		for (int i = 0; i < nfds; ++i) {
			int active_fd = events[i].data.fd;
			auto coroutine = fd_to_coroutine_[active_fd];
			assert(coroutine != nullptr);

			removeEvent(active_fd);

			//todo:有四类事件：1.可读，2.可写，3.关闭，4.错误 需要处理
			coroutine->setState(CoroutineState::RUNNABLE);
			processer_->addTask(coroutine);
		}
		Coroutine::SwapOut();
	}
}
```

##### 为什么需要一个wake协程
可能出现这种情况：正在执行Poll协程，并且没有事件到达，这时新加入一个协程，如果没有机制将Poll协程从poll()函数中唤醒，那么这个新的协程将无法得到执行。wake协程会read eventfd，此时会将<eventfd, wake协程>注册到Poller中，如果有新的协程加入，会往eventfd写1字节的数据，那么poll()函数就会被唤醒，从而Poll协程让出CPU，新加入的协程被调度。

### 定时器
#### 原理
``` c++
#include <sys/timerfd.h>
int timerfd_create(int clockid, int flags); //创建一个timer对象，返回一个文件描述符timer fd代表这个timer对象。
int timerfd_settime(int fd, int flags,
                           const struct itimerspec *new_value,
                           struct itimerspec *old_value);  //为timer对象设置一个时间间隔，倒计时结束后timer fd将变为可读。
```

![定时器](https://blog-1253119293.cos.ap-beijing.myqcloud.com/other/melon_github_readme/%E5%AE%9A%E6%97%B6%E5%99%A8.png)

1. 定时器专门占用一个线程。这个线程中加入一个定时器协程，该协程会去读取timer fd，可读后说明有定时器超时，然后执行定时器对应的任务。
2. TimerManager维护一个定时器队列。每一项包含定时器触发时间和对应的回调。
3. TimerManager.addTimer()将新的<timer, 回调>加入到队列中。如果这个定时器是最先到期的那么调用timerfd_settime()重新设置timer fd的到期时间。timer fd到期后，将从Poll协程中返回，然后执行定时器协程，该协程中读取timer fd，然后根据现在的时间，将定时器队列中超时的项删除，并将超时的项的回调作为新的协程执行。
4. 这个队列可以由multimap来实现，multimap由红黑树实现，内部是有序的。红黑树本质就是一颗二叉树，只不过为了防止多次的操作变得不平衡，增加了一些维持平衡的操作。
5. 如何删除定时器，每个定时器分配一个id，TimerManager内部维护一个id到定时器时间戳的映射sequence_2_timestamp_.cancel()时，根据id去sequence_2_timestamp_中找有没有对应的定时器，如果有，将这个时间戳从时间戳队列中删除，必要时重新调用timerfd_settime()。

```c++
int main() {
	Scheduler scheduler;
	scheduler.startAsync();

	/**
	 *  当timer_fd没有cancel之前，会每隔2s执行以下timeout回调，休眠10scancel，测试执行4次回调
	 *  当第一个参数为0时，只调用一次回调
	 */ 
	int64_t timer_id = scheduler.runEvery(2 * Timestamp::kMicrosecondsPerSecond, std::make_shared<Coroutine>([](){
									printf("timeout\n");
								}));
	sleep(10);  
	scheduler.cancel(timer_id);
	printf("cancel\n");

	return 0;
}
```

### Hook
要想实现在协程中遇到耗时操作不阻塞当前IO线程，需要对一些系统函数进行hook。
1. 可以用dlsym(3)来获取想要hook的函数的函数指针，先保存起来，如果想要用到原函数，可以通过保存的函数指针进行调用。
2. 定义自己的同名函数，覆盖想要hook的函数。以sleep(3)为例。
``` c++
unsigned int sleep(unsigned int seconds) {
	leo::Processer* processer = leo::Processer::GetProcesserOfThisThread();
	if (!leo::isHookEnabled()) {
		return sleep_f(seconds);
	}

	leo::Scheduler* scheduler = processer->getScheduler();
	assert(scheduler != nullptr);
	scheduler->runAt(leo::Timestamp::now() + seconds * leo::Timestamp::kMicrosecondsPerSecond, leo::Coroutine::GetCurrentCoroutine());
	leo::Coroutine::SwapOut();
	return 0;
}
```
我们自己定义的sleep不会阻塞线程，而是将当前协程切出去，让CPU执行其它协程，等时间到了再执行当前协程。这样就模拟了sleep的操作，同时不会阻塞当前线程。

### RPC实现
#### 参数序列化及反序列化
rpc说简单点就是将参数传给服务端，服务端根据参数找到对应的函数执行，得出一个响应，再将响应传回给客户端。客户端的参数对象如何通过网络传到服务端呢？这就涉及到序列化和反序列化。
leo不想使用第三方库的依赖，所以不选择用比较成熟的protobuf，二采用自定义序列化和反序列化的类Serializer，可序列化反序列化基本类型，具体运用如下：
``` c++
struct Student {
    int age;
    std::string name;
    Student() {}
    Student(int a, const std::string& n) 
        : age(a), name(n) {}
    
    friend Serializer& operator>>(Serializer& in, Student& s)
    {
        in >> s.age >> s.name;
        return in;
    }
    friend Serializer& operator<<(Serializer& out, Student& s)
    {
        out << s.age << s.name;
        return out;
    }
};

int main(int argc, char** argv)
{
    Serializer sr;
    /* 基本类型 */
    int n = 24;
    int v;
    sr << n;  // 序列化
    sr >> v;  // 反序列化
    cout << v << endl;

    /* 自定义类型类型 */
    Student src(23, "evenleo");
    Student dest;
    sr << src;
    sr >> dest;
    cout << dest.name << ", " << dest.age << endl;

    return 0;
}
```
上述测试成功实现基本类型的序列化反序列化的功能，

RPC服务端
```c++
std::string Strcat(std::string s, int n)
{
    return s + std::to_string(n);
}

struct Foo {
    int add(int a, int b) {
        return a + b;
    }
};

int main(int argc, char** argv)
{
    Singleton<Logger>::getInstance()->addAppender("console", LogAppender::ptr(new ConsoleAppender()));
    RpcServer server(5000, 3);
    server.regist("Strcat", Strcat);
    Foo s;
	server.regist("add", &Foo::add, &s);
    server.run();
    return 0;
}
```
RPC客户端
```c++
void StrcatResult(string responese)
{
    Serializer s(responese.c_str(), responese.size());
    response_t<string> res;
    s >> res;
    cout << "code=" << res.code() << ", message=" << res.message() << ", value=" << res.value() << endl;
}

void addResult(string responese)
{
    Serializer s(responese.c_str(), responese.size());
    response_t<int> res;
    s >> res;
    cout << "code=" << res.code() << ", message=" << res.message() << ", value=" << res.value() << endl;
}

int main(int argc, char** argv)
{
    RpcClient client("127.0.0.1", 5000);
    client.call<string>("Strcat", StrcatResult, "even", 24);
    client.call<int>("add", addResult, 10, 21);
    getchar();
    return 0;
}
```
某次rpc的过程如下：
```
启动服务器                ---------------->     注册service::method

客户端包装请求并发送       ---------------->     服务端解析请求，找到并执行对应的service::method

客户端接收响并解析结果     <----------------     服务端将响应发回给客户端
```
返回结果如下：
```
code=0, message=success, value=even24
code=0, message=success, value=31
```

### 一些细节
1. 需要开启SO_REUSEADDR选项。
2. 需要屏蔽SIG_PIPE信号。

本项目基于开源项目 [gatsbyd/melon](https://github.com/gatsbyd/melon) 的基础上开发，将poll替换成epoll，理论上更高效，但目前也没有进行压测对比。并增加了读取配置文件的Config类，可实现基本读取配置文件的功能。RPC功能实现不采用protobuf进行序列化和反序列化，而是采用自定义的基本类型的序列化反序列化的功能。这里非常感谢gatsbyd。
