#ifndef _LEO_HTTP_PARSER_H_
#define _LEO_HTTP_PARSER_H_

#include "http/Http.h"

#include <memory>

namespace leo {
namespace http {


class HttpParser {
public:
	typedef std::shared_ptr<HttpParser> ptr;
	HttpParser() = default;

	static int parseRequest(HttpRequest& request, const char* buf, size_t len);
};

}
}

#endif
