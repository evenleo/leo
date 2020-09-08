#include "EventPoller.h"
#include "Log.h"
#include "Processer.h"

#include <error.h>
#include <assert.h>
#include <string.h>
#include <sstream>

namespace leo {

EventPoller::EventPoller(Processer* processer)
	: is_polling_(false), processer_(processer) {
	epfd_ = epoll_create1(0);  //flag=0 等价于epll_craete
	if (epfd_ < 0) {
		LOG_ERROR << "Failed to create epoll";
		exit(1);
	}
}	

void EventPoller::updateEvent(int fd, int events, Coroutine::ptr coroutine) {
	assert(coroutine != nullptr);
	auto it = fd_to_events_.find(fd);
	if (it == fd_to_events_.end()) {
		struct epoll_event e;
		e.data.fd = fd;
		e.events = events;
		fd_to_events_[fd] = e;
		fd_to_coroutine_[fd] = coroutine;
		if (epoll_ctl(epfd_, EPOLL_CTL_ADD, fd, &e) < 0) {
			LOG_ERROR << "Failed to insert handler to epoll";
		}
	} else {
		struct epoll_event& e = fd_to_events_[fd];
		e.events = events;
		fd_to_coroutine_[fd] = coroutine;
		epoll_ctl(epfd_, EPOLL_CTL_MOD, fd, &e);
	}
}
	
void EventPoller::removeEvent(int fd) {
	auto it = fd_to_events_.find(fd);
	if (it != fd_to_events_.end()) {
		LOG_INFO << "removeEvent fd=" << fd;
		fd_to_coroutine_.erase(fd);
		fd_to_events_.erase(fd);
		epoll_ctl(epfd_, EPOLL_CTL_DEL, fd, NULL);
	} 
}

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

}
