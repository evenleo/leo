#include "assert.h"
#include "Coroutine.h"
#include "Processer.h"
#include "Hook.h"
#include "Log.h"
#include "Timer.h"
#include "Scheduler.h"
#include "Socket.h"

#include <dlfcn.h>
#include <errno.h>
#include <memory>
#include <string.h>

namespace leo {

#define DLSYM(name) \
		name ## _f = (name ## _t)::dlsym(RTLD_NEXT, #name);

struct HookIniter {
	HookIniter() {
		DLSYM(sleep);
		DLSYM(usleep);
		DLSYM(accept);
		DLSYM(accept4);
		DLSYM(connect);
		DLSYM(read);
		DLSYM(readv);
		DLSYM(recv);
		DLSYM(recvfrom);
		DLSYM(recvmsg);
		DLSYM(write);
		DLSYM(writev);
		DLSYM(send);
		DLSYM(sendto);
		DLSYM(sendmsg);
	}
};

static HookIniter hook_initer;

static thread_local bool t_hook_enabled = false;

bool isHookEnabled() {
	return t_hook_enabled;
}

void setHookEnabled(bool flag) {
	t_hook_enabled = flag;
}

}

template<typename OriginFun, typename... Args>
static ssize_t ioHook(int fd, OriginFun origin_func, int event, Args&&... args) {
	ssize_t n;

	leo::Processer* processer = leo::Processer::GetProcesserOfThisThread();
	if (!leo::isHookEnabled()) {
		return origin_func(fd, std::forward<Args>(args)...);
	}

retry:
	do {
		n = origin_func(fd, std::forward<Args>(args)...);
	} while (n == -1 && errno == EINTR);

	if (n == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {

		//注册事件，事件到来后，将当前上下文作为一个新的协程进行调度
		processer->updateEvent(fd, event, leo::Coroutine::GetCurrentCoroutine());
		leo::Coroutine::GetCurrentCoroutine()->setState(leo::CoroutineState::BLOCKED);
		leo::Coroutine::SwapOut();

		goto retry;
	}

	return n;
}

extern "C" {
#define HOOK_INIT(name) \
		name ## _t name ## _f = nullptr;

HOOK_INIT(sleep)
HOOK_INIT(usleep)
HOOK_INIT(accept)
HOOK_INIT(accept4)
HOOK_INIT(connect)
HOOK_INIT(read)
HOOK_INIT(readv)
HOOK_INIT(recv)
HOOK_INIT(recvfrom)
HOOK_INIT(recvmsg)
HOOK_INIT(write)
HOOK_INIT(writev)
HOOK_INIT(send)
HOOK_INIT(sendto)
HOOK_INIT(sendmsg)


unsigned int sleep(unsigned int seconds) {
	leo::Processer* processer = leo::Processer::GetProcesserOfThisThread();
	if (!leo::isHookEnabled()) {
		return sleep_f(seconds);
	}

	leo::Scheduler* scheduler = processer->getScheduler();
	assert(scheduler != nullptr);
	scheduler->runAt(leo::Timer::getCurrentMs() + seconds * leo::kMicrosecondsPerSecond, leo::Coroutine::GetCurrentCoroutine());
	leo::Coroutine::SwapOut();
	return 0;
}

unsigned int usleep(uint64_t us) {
	leo::Processer* processer = leo::Processer::GetProcesserOfThisThread();
	if (!leo::isHookEnabled()) {
		return usleep_f(us);
	}

	leo::Scheduler* scheduler = processer->getScheduler();
	assert(scheduler != nullptr);
	scheduler->runAt(leo::Timer::getCurrentMs() + us, leo::Coroutine::GetCurrentCoroutine());
	leo::Coroutine::SwapOut();
	return 0;
}

int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen) {
	return ioHook(sockfd, accept_f, leo::Poller::kReadEvent, addr, addrlen);
}

int accept4(int sockfd, struct sockaddr *addr, socklen_t *addrlen, int flags) {
	return ioHook(sockfd, accept4_f, leo::Poller::kReadEvent, addr, addrlen, flags);
}

int connect(int sockfd, const struct sockaddr *addr,
				                   socklen_t addrlen) {
	leo::Processer* processer = leo::Processer::GetProcesserOfThisThread();
	if (!leo::isHookEnabled()) {
		return connect_f(sockfd, addr, addrlen);
	}

	int ret = ::connect_f(sockfd, addr, addrlen);

	if (ret == -1 && errno == EINPROGRESS) {
		processer->updateEvent(sockfd, leo::Poller::kWriteEvent, leo::Coroutine::GetCurrentCoroutine());
		leo::Coroutine::GetCurrentCoroutine()->setState(leo::CoroutineState::BLOCKED);
		leo::Coroutine::SwapOut();

		int err = leo::Socket::GetSocketError(sockfd);
		if (err == 0) {
			return 0;
		} else {
			return -1;
		}
	}
	
	return ret;
}

ssize_t read(int fd, void *buf, size_t count) {
	return ioHook(fd, read_f, leo::Poller::kReadEvent, buf, count);
}

ssize_t readv(int fd, const struct iovec *iov, int iovcnt) {
	return ioHook(fd, readv_f, leo::Poller::kReadEvent, iov, iovcnt);
}

ssize_t recv(int sockfd, void *buf, size_t len, int flags) {
	return ioHook(sockfd, recv_f, leo::Poller::kReadEvent, buf, len, flags);
}

ssize_t recvfrom(int sockfd, void *buf, size_t len, int flags, 
				struct sockaddr *src_addr, socklen_t *addrlen) {
	return ioHook(sockfd, recvfrom_f, leo::Poller::kReadEvent, buf, len, flags, src_addr, addrlen);
}

ssize_t recvmsg(int sockfd, struct msghdr *msg, int flags) {
	return ioHook(sockfd, recvmsg_f, leo::Poller::kReadEvent, msg, flags);
}

ssize_t write(int fd, const void *buf, size_t count) {
	return ioHook(fd, write_f, leo::Poller::kWriteEvent, buf, count);
}


ssize_t writev(int fd, const struct iovec *iov, int iovcnt) {
	return ioHook(fd, writev_f, leo::Poller::kWriteEvent, iov, iovcnt);
}

ssize_t send(int sockfd, const void *buf, size_t len, int flags) {
	return ioHook(sockfd, send_f, leo::Poller::kWriteEvent, buf, len, flags);
}

ssize_t sendto(int sockfd, const void *buf, size_t len, int flags, 
		const struct sockaddr *dest_addr, socklen_t addrlen) {
	return ioHook(sockfd, sendto_f, leo::Poller::kWriteEvent, buf, len, flags, dest_addr, addrlen);
}

ssize_t sendmsg(int sockfd, const struct msghdr *msg, int flags) {
	return ioHook(sockfd, sendmsg_f, leo::Poller::kWriteEvent, msg, flags);	
}

}



