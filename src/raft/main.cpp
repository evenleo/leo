#include <iostream>
#include "raft.h"


int main(int argc, char** argv) 
{
    Singleton<Logger>::getInstance()->addAppender("console", LogAppender::ptr(new ConsoleAppender()));
    int port = atoi(argv[1]);
    Raft rt(port, port);
    rt.start();

    std::vector<Address> addresses;
    addresses.push_back({"127.0.0.1", 5001});
    addresses.push_back({"127.0.0.1", 5002});
    addresses.push_back({"127.0.0.1", 5003});

    rt.addPeers(addresses);

    // rt.peer("127.0.0.1", 5001);


    getchar();

    rt.rescheduleElection();
    // rt.voteTest("127.0.0.1", 5001);

    getchar();

    std::cout << "close" << std::endl;
    rt.close();

    getchar();

    getchar();
    getchar();


    return 0;
}