#ifndef _LEO_TIME_STAMP_H_
#define _LEO_TIME_STAMP_H_

#include <stdint.h>
#include <string>
#include <sys/time.h>
#include <errno.h>
#include <string.h>

namespace leo {

class Timestamp {
public:
    const static uint64_t kMicrosecondsPerSecond = 1000 * 1000;

    Timestamp(): microseconds_from_epoch_(0) {}

    explicit Timestamp(uint64_t microseconds_from_epoch)
        : microseconds_from_epoch_(microseconds_from_epoch) {}

    uint64_t getMicroSecondsFromEpoch() { return microseconds_from_epoch_; }

    time_t getSec() const { return microseconds_from_epoch_ / kMicrosecondsPerSecond; }

    suseconds_t getUsec() const { return microseconds_from_epoch_ % kMicrosecondsPerSecond; }

    static Timestamp now() {
        struct timeval tv;
        if (gettimeofday(&tv, nullptr)) {
            std::cerr << "gettimeofday:" << strerror(errno) << std::endl;
        }
        return Timestamp(tv.tv_sec * kMicrosecondsPerSecond + tv.tv_usec);
    }

private:
    uint64_t microseconds_from_epoch_;
};

inline bool operator<(Timestamp lhs, Timestamp rhs)
{
    return lhs.getMicroSecondsFromEpoch() < rhs.getMicroSecondsFromEpoch();
}

inline bool operator==(Timestamp lhs, Timestamp rhs)
{
    return lhs.getMicroSecondsFromEpoch() == rhs.getMicroSecondsFromEpoch();
}

inline Timestamp operator+(Timestamp lhs, uint64_t micro_seconds)
{
    return Timestamp(lhs.getMicroSecondsFromEpoch() + micro_seconds);
}

inline int64_t operator-(Timestamp lhs, Timestamp rhs)
{
    return lhs.getMicroSecondsFromEpoch() - rhs.getMicroSecondsFromEpoch();
}

inline std::ostream& operator<<(std::ostream& os, const Timestamp& timestamp)
{
    char buf[50];
    time_t sec = timestamp.getSec();
    struct tm tm;
    localtime_r(&sec, &tm);
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tm);
    os << std::string(buf);
    return os;
}

}
#endif
