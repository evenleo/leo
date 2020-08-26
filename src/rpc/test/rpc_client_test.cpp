#include "rpc/Rpc.h"
#include <unistd.h>

using namespace leo;

void result(std::string responese) {
    Serializer s(responese.c_str(), responese.size());
    response_t<std::string> res;
    s >> res;
    std::cout << "res.val: " << res.val() << std::endl;
}

int main(int argc, char** argv)
{
    RpcClient client("127.0.0.1", 5000);
    client.call<std::string>("Strcat", result, "even", 24);
   
    while (1)
    {
        sleep(1);
    }
    return 0;
}