#include "rpc/Rpc.h"
#include <unistd.h>
#include "Log.h"

using namespace leo;
using namespace std;

int main(int argc, char** argv)
{
    Singleton<Logger>::getInstance()->addAppender("console", LogAppender::ptr(new ConsoleAppender()));
    RpcClient client("127.0.0.1", 5000);
    response_t<int> res = client.call<int>("add", 10, 21);
    cout << "code=" << res.code() << ", message=" << res.message() << ", value=" << res.value() << endl;
    response_t<string> r = client.call<string>("Strcat", "even", 24);
    cout << "code=" << r.code() << ", message=" << r.message() << ", value=" << r.value() << endl;

    getchar();
    
    response_t<int> res2 = client.call<int>("add", 100, 21);
    cout << "code=" << res2.code() << ", message=" << res2.message() << ", value=" << res2.value() << endl;

    return 0;
}