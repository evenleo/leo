#include "rpc/RpcServer.h"
#include "Log.h"
#include "Scheduler.h"

#include "echo.pb.h"


// protoc --cpp_out=./ ./echo.proto 

using namespace leo;
using namespace leo::rpc;

MessagePtr onEcho(std::shared_ptr<echo::EchoRequest> request) {
	LOG_INFO << "server receive request, message:" << request->msg();
	std::shared_ptr<echo::EchoResponse> response(new echo::EchoResponse);
	response->set_msg(request->msg());
	return response;
}

// int main(int argc, char** argv)
// {
//     Singleton<Logger>::getInstance()->addAppender("console", LogAppender::ptr(new ConsoleAppender()));
//     RpcServer server(5000, 3);
//     server.regist("Strcat", Strcat);
//     Foo s;
// 	server.regist("add", &Foo::add, &s);
//     server.run();
//     return 0;
// }

int main(int argc, char** argv) {
	int port = argc > 1 ? atoi(argv[1]) : 5000;
	Logger::setLogLevel(LogLevel::INFO);
	Singleton<Logger>::getInstance()->addAppender("console", LogAppender::ptr(new ConsoleAppender()));
	Scheduler scheduler;
	scheduler.startAsync();
	IpAddress addr(port);
	RpcServer server(addr, &scheduler);

	server.registerRpcHandler<echo::EchoRequest>(onEcho);
	// server.registerRpcHandler<RequestAppendArgs>(onAppendEntry);

	server.start();
	getchar();
	return 0;
}
