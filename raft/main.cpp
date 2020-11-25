#include <iostream>
#include "raft.h"


int main(int argc, char** argv) 
{
    // std::cout << isLittleEndian() << std::endl;
    Singleton<Logger>::getInstance()->addAppender("console", LogAppender::ptr(new ConsoleAppender()));

    // Serializer sr;
    // AppendEntryArgs args(10, 9, 8, 7, 6);
    // sr << args;

    // AppendEntryArgs b;
    // sr >> b;
    // std::cout << b.term_ << ", " << b.id_ << std::endl;


    // getchar();
    
    int port = argc > 0 ? atoi(argv[1]) : 5001;
    Raft rt(port, port);

    std::vector<Address> addresses;
    addresses.push_back({"127.0.0.1", 5001});
    addresses.push_back({"127.0.0.1", 5002});
    addresses.push_back({"127.0.0.1", 5003});

    rt.addPeers(addresses);
   

    getchar();
    rt.start();
    getchar();


    // rt.rescheduleElection();
    // // rt.voteTest("127.0.0.1", 5001);

    // getchar();

    // std::cout << "close" << std::endl;
    // rt.close();

    // getchar();

    // getchar();
    // getchar();


    return 0;
}