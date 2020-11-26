#include <iostream>
#include <thread>
#include "raft.h"


int main(int argc, char** argv) 
{
    // std::cout << isLittleEndian() << std::endl;
    Singleton<Logger>::getInstance()->addAppender("console", LogAppender::ptr(new ConsoleAppender()));

    int port = argc > 0 ? atoi(argv[1]) : 5001;
    
    Raft rt(port, port);
    std::vector<Address> addresses;
    addresses.push_back({"127.0.0.1", 5001});
    addresses.push_back({"127.0.0.1", 5002});
    addresses.push_back({"127.0.0.1", 5003});

    rt.addPeers(addresses);
    rt.start();


    getchar();
    

    getchar();
    
    getchar();


    return 0;
}