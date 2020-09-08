#ifndef _MELON_EVENT_POLLER_H_
#define _MELON_EVENT_POLLER_H_

#include "Noncopyable.h"
#include "Coroutine.h"
#include "Poller.h"

#include <map>
#include <memory>
#include <vector>
#include <sys/epoll.h>
#include <sys/socket.h>

namespace leo {

class Processer;

class EventPoller : public Poller {
public:
	EventPoller(Processer* scheduler);

	void updateEvent(int fd, int events, Coroutine::ptr coroutine) override;
	void removeEvent(int fd) override;

	void poll(int timeout) override;
	bool isPolling() { return is_polling_; }
	void setPolling(bool polling) { is_polling_ = polling; }
private:
	bool is_polling_;
	std::vector<struct epoll_event> epevents_;
	std::map<int, Coroutine::ptr> fd_to_coroutine_;
	std::map<int, size_t> fd_to_index_;
	Processer* processer_;
	int epfd_;
};

}

#endif
