#include "TcpServer.h"
#include "Log.h"
#include "Scheduler.h"

#include <unistd.h>
#include <stdio.h>

using namespace leo;

void handleClient(TcpConnection::ptr conn){
	conn->setTcpNoDelay(true);
	Buffer::ptr buffer = std::make_shared<Buffer>();
	while (conn->read(buffer) > 0) {
		conn->write(buffer);
	}
	conn->close();
}

int main(int argc, char* argv[]) {
	Singleton<Logger>::getInstance()->addAppender("console", LogAppender::ptr(new ConsoleAppender()));
	if (argc < 2) {
		LOG_ERROR << "please input thread num!";
	}
	IpAddress listen_addr(5000);
	int threads_num = atoi(argv[1]);
	Scheduler scheduler(threads_num);
	scheduler.startAsync();
	TcpServer server(listen_addr, &scheduler);
	server.setConnectionHandler(handleClient);
	server.start();

	scheduler.wait();
	return 0;
}
