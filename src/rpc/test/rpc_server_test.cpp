#include "rpc/Rpc.h"

using namespace leo;


int main(int argc, char** argv)
{
    RpcServer server(5000, 3);
    server.run();
    return 0;
}