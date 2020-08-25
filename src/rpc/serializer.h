#ifndef SERIALIZER_H
#define SERIALIZER_H

#include <vector>
#include <algorithm>
#include "Buffer.h"
#include "Endian.h"

namespace leo {

// 存储数据流的容器
class StreamBuffer : public std::vector<char> {
public:
    StreamBuffer() : curpos_(0) {}
    StreamBuffer(const char* s, size_t len) : curpos_(0)
    {
        insert(begin(), s, s + len);
    }
    const char* data() { return &(*this)[0]; }
    const char* curdata() { return &(*this)[curpos_]; }
    size_t cursize() const { return size() - curpos_; }
    bool eof() const { return curpos_ >= size(); }
    void offset(int k) { curpos_ += k; }
    void input(const char* s, size_t len) { insert(end(), s, s + len); }
    void reset() { curpos_ = 0; }

private:
    size_t curpos_;
};

class Serializer {
public:
    Serializer() : {}
    Serializer(Buffer::ptr buffer)
        : buffer_(buffer) {}
    
    template <typename T>
    void input_type(T t);
    
    template <typename T>
    void output_type(T& t);

    template<typename Tuple, std::size_t Id>
	void getv(Serializer& ds, Tuple& t) {
		ds >> std::get<Id>(t);
	}

    void reset(){
		streamBuffer_.reset();
	}

    void clear() 
    { 
        streamBuffer_.clear();
        reset();
    }
    void input(const char* data, int len)
    {
        streamBuffer_.input(data, len);
    }

    template<typename Tuple, std::size_t... I>
	Tuple get_tuple(std::index_sequence<I...>) {
		Tuple t;
		((getv<Tuple, I>(*this, t)), ...);
		return t;
	}

    template<typename T>
	Serializer& operator >> (T& i)
    {
		output_type(i); 
		return *this;
	}

	template<typename T>
	Serializer& operator << (T i)
    {
		input_type(i);
		return *this;
	}
    
    const char* data() { return buffer_->peek(); }
    size_t size() const { return buffer_->readableBytes(); }


private:
    int byteOrder_;
    Buffer::ptr buffer_;
};

template <typename T>
inline void Serializer::input_type(T t)
{
    T v = byteswap(t);
    buffer_->append(&v, sizeof(v));
}

// 偏特化
template <>
inline void Serializer::input_type(std::string t)
{
    // 先存入字符串长度
    uint16_t len = t.size();
    buffer_->appendInt16(len);
    // 存入字符串
    buffer_->append(t.c_str(), len);
}

template<>
inline void Serializer::input_type(const char* in)
{
	input_type<std::string>(std::string(in));
}

template <typename T>
inline void Serializer::output_type(T& t)
{
    assert(buffer_->readableBytes() > sizeof(t));
    int len = sizeof(T);
    char* d = new char[len];
    if (!streamBuffer_.eof())
    {
        memcpy(d, streamBuffer_.curdata(), len);
        streamBuffer_.offset(len);
        byte_order(d, len);
        t = *reinterpret_cast<T*>(&d[0]);
    }
    delete[] d;
}

// 偏特化
template <>
inline void Serializer::output_type(std::string& t)
{
    // 先取字符串长度
    int strlen = sizeof(uint16_t);
    char* d = new char[strlen];
    memcpy(d, streamBuffer_.curdata(), strlen);
    byte_order(d, strlen);
    streamBuffer_.offset(strlen);
    int len = *reinterpret_cast<uint16_t*>(&d[0]);
    delete[] d;

    // 再取字符串
    if (len == 0) return;
    t.insert(t.begin(), streamBuffer_.curdata(), streamBuffer_.curdata() + len);
    streamBuffer_.offset(len);
}

}

#endif