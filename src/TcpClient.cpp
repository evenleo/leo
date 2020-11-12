#include "Scheduler.h"
#include "TcpClient.h"
#include  "Log.h"
#include "Hook.h"

#include <unistd.h>
#include <sys/socket.h>
#include <string.h>

namespace leo {

const uint64_t TcpClient::kMaxRetryDelayUs;

TcpClient::TcpClient(const IpAddress& server_addr)
	: server_addr_(server_addr) {
}

TcpConnection::ptr TcpClient::connect(uint64_t timeout_ms) {
	Timestamp start = Timestamp::now();
	uint64_t retry_delay_us = 10 * 1000;   // 休眠10ms开始
	while (1)
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
					|| errno == ENETUNREACH) {
			usleep(retry_delay_us);
			retry_delay_us = std::min(retry_delay_us * 2, kMaxRetryDelayUs);
			if (timeout_ms != (uint64_t)-1 
			    && Timestamp::now().getMicroSecondsFromEpoch() - start.getMicroSecondsFromEpoch() > timeout_ms * 1000) {
				// LOG_DEBUG << "timeout!";
				return nullptr;
			}
			// LOG_DEBUG << "TcpClient::connect to " << server_addr_.toString() << " retry";
		} else {	// failed
			LOG_ERROR << "connect error in TcpClinet::connect, " << strerror(errno) << ", hooked=" << isHookEnabled();
			return nullptr;
		}
	}
}

}
