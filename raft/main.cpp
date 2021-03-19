#include <iostream>
#include <thread>
#include "raft.h"

struct Address
{
    Address(const std::string &_ip, int _port)
        : ip(_ip), port(_port) {}
    std::string ip;
    int port;
};

int main(int argc, char** argv) 
{
    // std::cout << isLittleEndian() << std::endl;
    Singleton<Logger>::getInstance()->addAppender("console", LogAppender::ptr(new ConsoleAppender()));

    int port = argc > 0 ? atoi(argv[1]) : 5001;
    int me = port == 5001 ? 0 : (port == 5002 ? 1 : 2);

    std::vector<Address> addresses;
    addresses.push_back({"127.0.0.1", 5001});
    addresses.push_back({"127.0.0.1", 5002});
    addresses.push_back({"127.0.0.1", 5003});

    Scheduler::ptr scheduler = std::make_shared<Scheduler>();

    std::vector<RpcClient::ptr> peers;
    for (auto &x : addresses)
    {
        IpAddress server_addr(x.ip, x.port);
        peers.push_back(std::make_shared<RpcClient>(server_addr, scheduler.get()));
    }


    Raft rt(me, port, scheduler, peers);
   
    rt.start();

    getchar();
    

    getchar();
    
    getchar();


    return 0;
}