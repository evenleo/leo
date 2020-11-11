#include "Scheduler.h"
#include "TcpClient.h"
#include  "Log.h"
#include "Hook.h"

#include <unistd.h>
#include <sys/socket.h>
#include <string.h>

namespace leo {

const int TcpClient::kMaxRetryDelayUs;

TcpClient::TcpClient(const IpAddress& server_addr)
	: server_addr_(server_addr), timeout_id_(-1), isTimeout_(false) {
}

TcpConnection::ptr TcpClient::connect() {
retry:
	{
		Socket::ptr sock = Socket::CreateTcp();
		sock->SetNonBlockAndCloseOnExec();

		int ret = sock->connect(server_addr_);
		if (ret == 0) {
			return std::make_shared<TcpConnection>(sock, server_addr_);
		} else if (errno == EAGAIN 
					|| errno == EADDRINUSE 
					|| errno == EADDRNOTAVAIL 
					|| errno == ECONNREFUSED 
					|| errno == ENETUNREACH) {		//retry
			retry_delay_us_ = std::min(retry_delay_us_ * 2, kMaxRetryDelayUs);
			LOG_DEBUG << "sleep " << retry_delay_us_ << " ms";
			usleep(retry_delay_us_);
			LOG_DEBUG << "TcpClient::connect to " << server_addr_.toString() << " retry";
			goto retry;
		} else {	// failed
			LOG_ERROR << "connect error in TcpClinet::connect, " << strerror(errno) << ", hooked=" << isHookEnabled();
			return nullptr;
		}
		
	}
}

TcpConnection::ptr TcpClient::connect_with_timeout(uint64_t timeout_ms) {
retry:
	Socket::ptr sock = Socket::CreateTcp();
	sock->SetNonBlockAndCloseOnExec();

	int ret = sock->connect(server_addr_);
	if (ret == 0) {
		if (timeout_id_ != (uint64_t)-1) {
			leo::Processer* processer = leo::Processer::GetProcesserOfThisThread();
			processer->getScheduler()->cancel(timeout_id_);
		}
		return std::make_shared<TcpConnection>(sock, server_addr_);
	} else if (errno == EAGAIN 
				|| errno == EADDRINUSE 
				|| errno == EADDRNOTAVAIL 
				|| errno == ECONNREFUSED 
				|| errno == ENETUNREACH) {		//retry
		
		if (timeout_id_ == (uint64_t)-1) {
			leo::Processer* processer = leo::Processer::GetProcesserOfThisThread();
			timeout_id_ =  processer->getScheduler()->runAfter(timeout_ms, std::make_shared<Coroutine>([this]() { 
				MutexGuard guard(mutex_);
				isTimeout_ = true; 
				return;
			}));
		}
		LOG_DEBUG << "----------------------------------111111111111111111";
		usleep(1000 * 100);
		LOG_DEBUG << "----------------------------------222222222222222222";

		if (isTimeout_) {
			LOG_DEBUG << "TcpClient::connect to " << server_addr_.toString() << " timeout";
			return nullptr;
		}

		LOG_DEBUG << "TcpClient::connect to " << server_addr_.toString() << " retry";
		goto retry;
	} else {	// failed
		if (timeout_id_ != (uint64_t)-1) {
			leo::Processer* processer = leo::Processer::GetProcesserOfThisThread();
			processer->getScheduler()->cancel(timeout_id_);
		}
		LOG_ERROR << "connect error in TcpClinet::connect, " << strerror(errno) << ", hooked=" << isHookEnabled();
		return nullptr;
	}
}

}
