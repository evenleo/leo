#ifndef SERIALIZER_H
#define SERIALIZER_H

#include <vector>
#include <algorithm>
#include "Buffer.h"

namespace leo {

// 主机字节序是否小端字节序
static bool isLittleEndian()
{
    static uint16_t flag = 1;
    static bool little_end_flag = *((uint8_t*)&flag) == 1;
    return little_end_flag;
}

class Serializer {
public:
    Serializer() { buffer_ = std::make_shared<Buffer>(); }
    Serializer(Buffer::ptr buffer)
        : buffer_(buffer) {}
    
    template <typename T>
    void input_type(T t);
    
    template <typename T>
    void output_type(T& t);

    // template<typename Tuple, std::size_t Id>
	// void getv(Serializer& ds, Tuple& t) {
	// 	ds >> std::get<Id>(t);
	// }

    // void reset(){
	// 	streamBuffer_.reset();
	// }

    // void clear() 
    // { 
    //     streamBuffer_.clear();
    //     reset();
    // }
    // void input(const char* data, int len)
    // {
    //     streamBuffer_.input(data, len);
    // }

    // template<typename Tuple, std::size_t... I>
	// Tuple get_tuple(std::index_sequence<I...>) {
	// 	Tuple t;
	// 	((getv<Tuple, I>(*this, t)), ...);
	// 	return t;
	// }

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
    static void byteOrder(char* s, int len)
    {
        if (isLittleEndian())
            std::reverse(s, s + len);
    }

private:
    int byteOrder_;
    Buffer::ptr buffer_;
};

template <typename T>
inline void Serializer::input_type(T v)
{
    size_t len = sizeof(v);
    char* p = reinterpret_cast<char*>(&v);
    byteOrder(p, len);
    buffer_->append(p, len);
}

// 偏特化
template <>
inline void Serializer::input_type(std::string v)
{
    // 先存入字符串长度
    uint16_t len = v.size();
    input_type(len);
    byteOrder(const_cast<char*>(v.c_str()), len);
    // 再存入字符串
    buffer_->append(const_cast<char*>(v.c_str()), len); 
}

template<>
inline void Serializer::input_type(const char* v)
{
	input_type<std::string>(std::string(v));
}

template <typename T>
inline void Serializer::output_type(T& v)
{
    size_t len = sizeof(v);
    assert(size() >= len);
    ::memcpy(&v, data(), len);
    buffer_->retrieve(len);
    byteOrder(reinterpret_cast<char*>(&v), len);
}

// 偏特化
template <>
inline void Serializer::output_type(std::string& v)
{
    uint16_t strLen = 0;
    output_type(strLen);
    v = std::string(data(), strLen);
    buffer_->retrieve(strLen);
    byteOrder(const_cast<char*>(v.c_str()), strLen);
}

}

#endif