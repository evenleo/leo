#include "rpc/Rpc.h"
#include <unistd.h>

using namespace leo;
using namespace std;

void StrcatResult(string responese)
{
    Serializer s(responese.c_str(), responese.size());
    response_t<string> res;
    s >> res;
    cout << "code=" << res.code() << ", message=" << res.message() << ", value=" << res.value() << endl;
}

void addResult(string responese)
{
    Serializer s(responese.c_str(), responese.size());
    response_t<int> res;
    s >> res;
    cout << "code=" << res.code() << ", message=" << res.message() << ", value=" << res.value() << endl;
}

int main(int argc, char** argv)
{
    RpcClient client("127.0.0.1", 5000);
    client.call<string>("Strcat", StrcatResult, "even", 24);
    client.call<int>("add", addResult, 10, 21);
    getchar();
    return 0;
}