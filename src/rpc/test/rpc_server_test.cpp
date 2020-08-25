#include "rpc/Rpc.h"

using namespace leo;

std::string Strcat(std::string s1, int n) 
{
    return s1 + std::to_string(n);
}

int main(int argc, char** argv)
{
    RpcServer server(5000, 3);
    server.regist("Strcat", Strcat);
    server.run();
    return 0;
}