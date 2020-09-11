#include "Poller.h"
#include "Processer.h"
#include "Log.h"

#include <error.h>
#include <assert.h>
#include <string.h>
#include <sstream>

namespace leo {

const int Poller::kNoneEvent = 0;
const int Poller::kReadEvent = EPOLLIN | EPOLLPRI;
const int Poller::kWriteEvent = EPOLLOUT;
	
std::string Poller::eventToString(int event) {
  std::ostringstream oss;
  if (event & EPOLLIN)
    oss << "IN ";
  if (event & EPOLLPRI)
    oss << "PRI ";
  if (event & EPOLLOUT)
    oss << "OUT ";
  if (event & EPOLLHUP)
    oss << "HUP ";
  if (event & EPOLLRDHUP)
    oss << "RDHUP ";
  if (event & EPOLLERR)
    oss << "ERR ";

  return oss.str();
}

EventPoller::EventPoller(Processer* processer)
	: is_polling_(false), processer_(processer) {
	LOG_INFO << "EventPoller";
	epfd_ = epoll_create1(0);  //flag=0 等价于epll_craete
	if (epfd_ < 0) {
		LOG_ERROR << "Failed to create epoll";
		exit(1);
	}
}	

EventPoller::~EventPoller()
{
	LOG_INFO << "~EventPoller";
}

void EventPoller::updateEvent(int fd, int events, Coroutine::ptr coroutine) {
	LOG_INFO << "updateEvent events=" << events;
	assert(coroutine != nullptr);
	auto it = fd_to_events_.find(fd);
	if (it == fd_to_events_.end()) {
		epoll_event e;
		e.data.fd = fd;
		e.events = events;
		fd_to_events_[fd] = e;
		fd_to_coroutine_[fd] = coroutine;
		if (epoll_ctl(epfd_, EPOLL_CTL_ADD, fd, &e) < 0) {
			LOG_ERROR << "Failed to insert handler to epoll";
		}
	} else {
		epoll_event& e = fd_to_events_[fd];
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
			LOG_INFO << "events=" << events[i].events;
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