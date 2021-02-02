#include "rpc/RpcClient.h"
#include <unistd.h>
#include "Log.h"
#include "args.pb.h"

using namespace leo;
using namespace leo::rpc;
using namespace std;

int main(int argc, char** argv)
{
    if (argc < 2) {
		printf("Usage: %s ip\n", argv[0]);
		return 0;
	}

	Singleton<Logger>::getInstance()->addAppender("console", LogAppender::ptr(new ConsoleAppender()));

	IpAddress server_addr(argv[1], 5000);
	Scheduler scheduler;
	scheduler.startAsync();
	RpcClient client(server_addr, &scheduler);

	std::shared_ptr<args::AddRequest> request(new args::AddRequest);
	request->set_a(1);
	request->set_b(2);
	client.Call<args::AddResponse>(request, [](std::shared_ptr<args::AddResponse> response) {
		LOG_INFO << "client receive response, result:" << response->result();
	});

	getchar();

    return 0;
}