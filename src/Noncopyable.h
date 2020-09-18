#ifndef _LEO_NON_COPYABLE_H_
#define _LEO_NON_COPYABLE_H_

namespace leo {

class Noncopyable {
public:
	Noncopyable(const Noncopyable& rhs) = delete;
	Noncopyable& operator=(const Noncopyable& rhs) = delete;

protected:
	Noncopyable() = default;
	~Noncopyable() = default;

};

}

#endif
