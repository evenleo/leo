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

private:
	static const int kMaxRetryDelayMs = 15 * 1000;
	static const int kInitRetryDelayMs = 500;

	IpAddress server_addr_;
	int retry_delay_ms_ = kInitRetryDelayMs;
};

}

#endif
