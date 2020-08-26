#include "Timestamp.h"

#include <errno.h>
#include <iostream>
#include <string.h>
#include <sys/time.h>

namespace leo {

Timestamp Timestamp::now()
{
    struct timeval tv;
    if (gettimeofday(&tv, nullptr)) {
        std::cerr << "gettimeofday:" << strerror(errno) << std::endl;
    }
    return Timestamp(tv.tv_sec * kMicrosecondsPerSecond + tv.tv_usec);
}

time_t Timestamp::getSec() const
{
    return microseconds_from_epoch_ / kMicrosecondsPerSecond;
}

suseconds_t Timestamp::getUsec() const
{
    return microseconds_from_epoch_ % kMicrosecondsPerSecond;
}

}
