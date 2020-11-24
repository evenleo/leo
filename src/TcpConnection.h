#ifndef _LEO_TCP_CONNECTION_H_
#define _LEO_TCP_CONNECTION_H_

#include "Address.h"
#include "Buffer.h"
#include "Socket.h"

namespace leo {

class TcpConnection {
public:
	typedef std::shared_ptr<TcpConnection> ptr;

	explicit TcpConnection(Socket::ptr socket, IpAddress peer)
		: conn_socket_(socket), peer_addr_(peer) {}

	ssize_t read(void* buf, size_t count) { return conn_socket_->read(buf, count); }

	ssize_t readn(void* buf, size_t count);

	ssize_t read(Buffer::ptr buf) { return buf->readSocket(conn_socket_); }

	ssize_t write(const void* buf, size_t count) { return conn_socket_->write(buf, count); }

	ssize_t writen(const void* buf, size_t count);

	ssize_t write(Buffer::ptr);

	ssize_t write(const std::string& message) { return writen(message.data(), message.size());}

	void shutdown() { conn_socket_->shutdownWrite(); }

	void readUntilZero();

	void close() { conn_socket_->close(); }

	void setTcpNoDelay(bool on) { conn_socket_->setTcpNoDelay(on); }

	const IpAddress& peerAddr() const { return peer_addr_; }

private:
	Socket::ptr conn_socket_;
	IpAddress peer_addr_;
};

}

#endif
