#include "http/HttpConnection.h"
#include "TcpServer.h"
#include "Scheduler.h"
#include "Log.h"

#include <stdio.h>

using namespace leo;
using namespace leo::http;

int main() {
	Singleton<Logger>::getInstance()->addAppender("console", LogAppender::ptr(new ConsoleAppender()));

	Scheduler scheduler;
	scheduler.startAsync();
	IpAddress addr(80);
	TcpServer server(addr, &scheduler);
	server.setConnectionHandler([](TcpConnection::ptr conn) {
					HttpConnection::ptr http_conn = std::make_shared<HttpConnection>(conn);
					HttpRequest::ptr request = http_conn->recvRequest();

					HttpResponse::ptr rsp = std::make_shared<HttpResponse>();
					rsp->setHttpStatus(HttpStatus::OK);
					rsp->setHeader("Content-Length", "5");
					rsp->setContent("hello");
					http_conn->sendResponse(rsp);
				});

	server.start();
	getchar();
	return 0;
}
