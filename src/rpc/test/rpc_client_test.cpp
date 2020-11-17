#include "rpc/RpcClient.h"
#include <unistd.h>
#include "Log.h"

#include "echo.pb.h"

// #include <memory>

using namespace leo;
using namespace leo::rpc;
using namespace std;

int main(int argc, char** argv)
{
    if (argc < 2) {
		printf("Usage: %s ip\n", argv[0]);
		return 0;
	}
	// Logger::setLogLevel(LogLevel::INFO);
	Singleton<Logger>::getInstance()->addAppender("console", LogAppender::ptr(new ConsoleAppender()));

	IpAddress server_addr(argv[1], 5000);
	Scheduler scheduler;
	scheduler.startAsync();
	RpcClient client(server_addr, &scheduler);

   	std::shared_ptr<echo::EchoRequest> request(new echo::EchoRequest);
	request->set_msg("hello");
	client.Call<echo::EchoResponse>(request, [](std::shared_ptr<echo::EchoResponse> response) {
						LOG_INFO << "client receive response, message:" << response->msg();
					});

	getchar();
    return 0;
}