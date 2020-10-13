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

// void addResult(string responese)
// {
//     Serializer s(responese.c_str(), responese.size());
//     response_t<int> res;
//     s >> res;
//     cout << "code=" << res.code() << ", message=" << res.message() << ", value=" << res.value() << endl;
// }

int main(int argc, char** argv)
{
    RpcClient client("127.0.0.1", 5000);
    // client.call<string>("Strcat", StrcatResult, "even", 24);
    response_t<int> res = client.call<int>("add", 10, 21);
    cout << "code=" << res.code() << ", message=" << res.message() << ", value=" << res.value() << endl;
    response_t<string> r = client.call<string>("Strcat", "even", 24);
    cout << "code=" << r.code() << ", message=" << r.message() << ", value=" << r.value() << endl;
    
    getchar();
    return 0;
}