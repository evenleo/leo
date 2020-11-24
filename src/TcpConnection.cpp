#include "Buffer.h"
#include "TcpConnection.h"

namespace leo {

ssize_t TcpConnection::readn(void* buf, size_t count) {
	size_t readn = 0;
	size_t left = count;
	while (left > 0) {
		ssize_t n = read(static_cast<char*>(buf) + readn, left);
		if (n <= 0) {
			return readn;
		}
		readn += n;
		left -= n;
	}
	return count;
}

ssize_t TcpConnection::writen(const void* buf, size_t count) {
	size_t writen = 0;
	size_t left = count;
	while (left > 0) {
		ssize_t n = write(static_cast<const char*>(buf) + writen, left);
		if (n <= 0) {
			return writen;
		}
		writen += n;
		left -= n;
	}
	return count;
}

ssize_t TcpConnection::write(Buffer::ptr buf) {
	ssize_t n = writen(buf->peek(), buf->readableBytes());
	if (n > 0) {
		buf->retrieve(n);
	}
	return n;
}

void TcpConnection::readUntilZero() {
	Buffer::ptr buffer(new Buffer);
	while (read(buffer) > 0) {
		buffer->retrieveAll();
	}
	return;
}

}
