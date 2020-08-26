#include "rpc/Rpc.h"
#include <unistd.h>

using namespace leo;

void StrcatResult(std::string responese)
{
    Serializer s(responese.c_str(), responese.size());
    response_t<std::string> res;
    s >> res;
    std::cout << "code=" << res.code() << ", message=" << res.message() << ", value=" << res.value() << std::endl;
}

void addResult(std::string responese)
{
    Serializer s(responese.c_str(), responese.size());
    response_t<int> res;
    s >> res;
    std::cout << "code=" << res.code() << ", message=" << res.message() << ", value=" << res.value() << std::endl;
}

int main(int argc, char** argv)
{
    RpcClient client("127.0.0.1", 5000);
    client.call<std::string>("Strcat", StrcatResult, "even", 24);
    client.call<int>("add", addResult, 10, 21);

    while (1) {
        sleep(1);
    }
    return 0;
}