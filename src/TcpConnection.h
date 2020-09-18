#ifndef _LEO_TCP_CONNECTION_H_
#define _LEO_TCP_CONNECTION_H_

#include "Address.h"
#include "Buffer.h"
#include "Socket.h"

namespace leo {

class TcpConnection {
public:
	typedef std::shared_ptr<TcpConnection> ptr;
	explicit TcpConnection(Socket::ptr socket, IpAddress peer);

	ssize_t read(void* buf, size_t count);
	ssize_t readn(void* buf, size_t count);
	ssize_t read(Buffer::ptr);
	ssize_t write(const void* buf, size_t count);
	ssize_t writen(const void* buf, size_t count);
	ssize_t write(Buffer::ptr);
	ssize_t write(const std::string& message);
	void shutdown();
	void readUntilZero();
	void close();
	void setTcpNoDelay(bool on);

	const IpAddress& peerAddr() const { return peer_addr_; }
private:
	Socket::ptr conn_socket_;
	IpAddress peer_addr_;
};

}

#endif
