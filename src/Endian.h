/**
 * @brief 字节序操作函数(大端/小端)
 */

#ifndef __ENDIAN_H__
#define __ENDIAN_H__

#include <endian.h>
#include <stdint.h>

namespace leo
{

    /**
 * @brief 8字节类型的字节序转化
 */
    template <typename T>
    typename std::enable_if<sizeof(T) == sizeof(uint64_t), T>::type
    byteswap(T value)
    {
        return (T)be64toh(value);
    }

    /**
 * @brief 4字节类型的字节序转化
 */
    template <typename T>
    typename std::enable_if<sizeof(T) == sizeof(uint32_t), T>::type
    byteswap(T value)
    {
        return (T)be32toh(value);
    }

    /**
 * @brief 2字节类型的字节序转化
 */
    template <typename T>
    typename std::enable_if<sizeof(T) == sizeof(uint16_t), T>::type
    byteswap(T value)
    {
        return (T)be16toh(value);
    }

    template <typename T>
    typename std::enable_if<sizeof(T) == sizeof(int8_t), T>::type
    byteswap(T value)
    {
        return value;
    }

    const uint16_t us_flag = 1;

    // little_end_flag 表示主机字节序是否小端字节序
    const bool little_end_flag = *((uint8_t*)&us_flag) == 1;

    //小端到主机

    template <typename T>
    T le_to_host(T &from) {
        T to;
        uint8_t byteLen = sizeof(T);

        if (little_end_flag)
            return from;
        else
        {
            char *to_char = (char*)&to;
            char *from_char = (char*)&from;
            for (int i = 0; i < byteLen; i++)
                to_char[i] = from_char[byteLen - i - 1];
                //此处也可用移位操作来实现
            return to;
        }
    }

    //主机到小端
    template <typename T>
    T host_to_le(T &from)
    {
        return le_to_host(from);
    }

    //大端到主机

    template <typename T>
    T be_to_host(T &from)

    {

        T to;

        uint8_t byteLen = sizeof(T);

        if (!little_end_flag)
        {

            return from;
        }

        else
        {

            char *to_char = (char *)&to;

            char *from_char = (char *)&from;

            for (int i = 0; i < byteLen; i++)
            {

                to_char[i] = from_char[byteLen - i - 1];

                //此处也可用移位操作来实现
            }

            return to;
        }
    }

    //主机到大端

    template <typename T>
    T host_to_be(T &from)

    {

        return be_to_host(from);
    }

} // namespace leo
}

#endif
