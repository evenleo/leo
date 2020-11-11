#ifndef _LEO_TCP_CLIENT_T_
#define _LEO_TCP_CLIENT_H_

#include "TcpConnection.h"

namespace leo {

class TcpClient {
public:
	typedef std::shared_ptr<TcpClient> ptr;

	TcpClient(const IpAddress& server_addr);
	virtual ~TcpClient() {}

	TcpConnection::ptr connect();
	TcpConnection::ptr connect_with_timeout(uint64_t timeout_ms);

private:
	static const int kMaxRetryDelayUs = 10 * 1000 * 1000;
	static const int kInitRetryDelayUs = 500 * 1000;

	IpAddress server_addr_;
	int retry_delay_us_ = kInitRetryDelayUs;
	uint64_t timeout_id_;
	bool isTimeout_;
	Mutex mutex_;
};
}

#endif
