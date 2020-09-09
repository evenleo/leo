#include "rpc/Rpc.h"
#include "Log.h"

using namespace leo;

std::string Strcat(std::string s, int n)
{
    return s + std::to_string(n);
}

struct Foo {
    int add(int a, int b) {
        return a + b;
    }
};

int main(int argc, char** argv)
{
    Singleton<Logger>::getInstance()->addAppender("console", LogAppender::ptr(new ConsoleAppender()));
    RpcServer server(5000, 3);
    server.regist("Strcat", Strcat);
    Foo s;
	server.regist("add", &Foo::add, &s);
    server.run();
    return 0;
}