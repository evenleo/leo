#ifndef _LEO_POLLER_H_
#define _LEO_POLLER_H_

#include "Noncopyable.h"
#include "Coroutine.h"

#include <map>
#include <memory>
#include <vector>
#include <sys/epoll.h>

namespace leo {

class Processer;

class Poller : public Noncopyable {
public:
	typedef std::shared_ptr<Poller> ptr;
	static const int kNoneEvent;
	static const int kReadEvent;
	static const int kWriteEvent;

	Poller() = default;
	virtual ~Poller() {};
	virtual void updateEvent(int fd, int events, Coroutine::ptr coroutine) = 0;
	virtual void removeEvent(int fd) = 0;
	virtual void poll(int timeout) = 0;

protected:
	std::string eventToString(int event);
};

class EventPoller : public Poller {
public:
	EventPoller(Processer* scheduler);
	~EventPoller();
	void updateEvent(int fd, int events, Coroutine::ptr coroutine) override;
	void removeEvent(int fd) override;
	void poll(int timeout) override;
	bool isPolling() { return is_polling_; }
	void setPolling(bool polling) { is_polling_ = polling; }

private:
	bool is_polling_;
	std::map<int, epoll_event> events_;
	std::map<int, Coroutine::ptr> coroutines_;
	Processer* processer_;
	int epfd_;
};

}

#endif