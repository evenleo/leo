#include "rpc/RpcServer.h"
#include "Log.h"
#include "Scheduler.h"
#include "args.pb.h"

using namespace leo;
using namespace leo::rpc;

MessagePtr onAdd(std::shared_ptr<args::AddRequest> request) {
	LOG_INFO << "server receive request, a=" << request->a() << ", b=" << request->b();
	std::shared_ptr<args::AddResponse> response(new args::AddResponse);
	response->set_result(request->a() + request->b());
	return response;
}

int main(int argc, char** argv) {
	int port = argc > 1 ? atoi(argv[1]) : 5000;
	Logger::setLogLevel(LogLevel::INFO);
	Singleton<Logger>::instance()->addAppender("console", LogAppender::ptr(new ConsoleAppender()));
	Scheduler scheduler;
	scheduler.startAsync();
	IpAddress addr(port);
	RpcServer server(addr, &scheduler);
	server.registerHandler<args::AddRequest>(onAdd);
	server.start();

	getchar();

	return 0;
}
