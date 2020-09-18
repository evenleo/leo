#ifndef _LEO_BUFFER_H_
#define _LEO_BUFFER_H_

#include <assert.h>
#include <algorithm>
#include <endian.h>
#include <stdio.h>
#include <string.h>
#include <vector>

#include "Socket.h"

namespace leo {

// refer to muduo by Shuo Chen, 有刪除不要的功能
class Buffer {
public:
	typedef std::shared_ptr<Buffer> ptr;
	/// A buffer class modeled after org.jboss.netty.buffer.ChannelBuffer
	///
	/// @code
	/// +-------------------+------------------+------------------+
	/// | prependable bytes |  readable bytes  |  writable bytes  |
	/// |                   |     (CONTENT)    |                  |
	/// +-------------------+------------------+------------------+
	/// |                   |                  |                  |
	/// 0      <=      readerIndex   <=   writerIndex    <=     size
/// @endcode
	static const size_t kCheapPrepend = 8;
	static const size_t kInitialSize = 1024;
	explicit Buffer(size_t initialSize = kInitialSize)
		: buffer_(kCheapPrepend + initialSize),
		  read_index_(kCheapPrepend),
		  write_index_(kCheapPrepend) 
	{
		assert(readableBytes() == 0);
		assert(writableBytes() == initialSize);
		assert(prependableBytes() == kCheapPrepend);
	}
	size_t readableBytes() const {
		return write_index_ - read_index_;
	}
	size_t writableBytes() const {
		return buffer_.size() - write_index_;
	}
	size_t prependableBytes() const {
		return read_index_;
	}
	//retrieve
	void retrieve(size_t len) {
		assert(len <= readableBytes());
		if (len < readableBytes()) {
			read_index_ += len;
		} else {
			retrieveAll();
    	}
	}
	void retrieveAll() {
		read_index_ = kCheapPrepend;
		write_index_ = kCheapPrepend;
	}
	//peek
	const char* peek() const {
		return begin() + read_index_;
	}
	
	std::string peekAsString() const {
		std::string result(peek(), readableBytes());
		return result;
	}
	
	//append
	void append(const char* data, size_t len) {
		ensureWritableBytes(len);
		std::copy(data, data+len, beginWrite());
		hasWritten(len);
	}
	void append(const void* data, size_t len) {
		append(static_cast<const char*>(data), len);
	}
	
	//preppand
	void prepend(const void* data, size_t len) {
		assert(len <= prependableBytes());
		read_index_ -= len;
		const char* d = static_cast<const char*>(data);
		std::copy(d, d + len, begin() + read_index_);
	}
	ssize_t readSocket(Socket::ptr socket);

	void ensureWritableBytes(size_t len) {
		if (writableBytes() < len) {
			makeSpace(len);
		}
		assert(writableBytes() >= len);
	}
	char* beginWrite() {
		return begin() + write_index_;	
	}
	const char* beginWrite() const {
		return begin() + write_index_;	
	}
	void hasWritten(size_t len) {
		assert(len <= writableBytes());
		write_index_ += len;
	}
private:
	char* begin() {
		return &*buffer_.begin();
	}
	const char* begin() const {
		return &*buffer_.begin();
	}
	void makeSpace(size_t len) {
		if (writableBytes() + prependableBytes() < len + kCheapPrepend) {
			buffer_.resize(write_index_ + len);
		} else {
			assert(kCheapPrepend < read_index_);
			size_t readable = readableBytes();
			std::copy(begin() + read_index_,
					begin() + write_index_,
					begin() + kCheapPrepend);
			read_index_ = kCheapPrepend;
			write_index_ = read_index_ + readable;
			assert(readable == readableBytes());
		}
	}

	std::vector<char> buffer_;
	size_t read_index_;
	size_t write_index_;
};

}

#endif
