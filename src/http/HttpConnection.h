#ifndef _LEO_HTTP_CONNECTION_H
#define _LEO_HTTP_CONNECTION_H

#include "TcpConnection.h"
#include "Http.h"

#include <memory>
#include <functional>

namespace leo {
namespace http {
	
class HttpConnection {
public:
	typedef std::shared_ptr<HttpConnection> ptr;
	explicit HttpConnection(TcpConnection::ptr tcp_conn);
	HttpRequest::ptr recvRequest();
	void sendResponse(HttpResponse::ptr response);

private:
	TcpConnection::ptr tcp_conn_;
	std::unique_ptr<char, std::function<void (char*)> > buffer_;
};

}
}

#endif
