#include "rpc/Rpc.h"
#include <unistd.h>

using namespace leo;



int main(int argc, char** argv)
{
    RpcClient client("127.0.0.1", 5000);
    sleep(1);
    std::cout << "begin" << std::endl;
    std::string s = client.call<std::string>("Strcat", "even", 24).val();
    std::cout << "end" << std::endl;

	std::cout << "call Strcat result: " << s << std::endl;

    while (1)
    {
        sleep(1);
    }
    return 0;
}