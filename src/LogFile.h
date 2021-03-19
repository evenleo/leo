#ifndef _LEO_LOG_FILE_H_
#define _LEO_LOG_FILE_H_

#include "Noncopyable.h"

#include <string>


namespace leo {

class LogFile : public Noncopyable {
public:
	LogFile(std::string basename);
	~LogFile();

	void persist(const char* data, size_t length);
	void flush();

private:
	std::string getFileName();
	static const int kMaxFileSize = 1024 * 1024 * 1024;

	std::string basename_;
	int bytes_writed_;
	FILE* fp_;
};

}

#endif
