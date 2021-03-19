#ifndef _LEO_TCP_CLIENT_T_
#define _LEO_TCP_CLIENT_H_

#include "TcpConnection.h"

namespace leo {

class TcpClient {
public:
	typedef std::shared_ptr<TcpClient> ptr;

	TcpClient(const IpAddress& server_addr);
	
	virtual ~TcpClient() {}

	TcpConnection::ptr connect(uint64_t timeout_ms = -1);

private:
	static const uint64_t kMaxRetryDelayUs = 10 * 1000 * 1000;
	static const uint64_t kInitRetryDelayUs = 500 * 1000;

	IpAddress server_addr_;
};
}

#endif
