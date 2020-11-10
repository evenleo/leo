#include <iostream>
#include "raft.h"


int main(int argc, char** argv) 
{
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

    rt.sendRequestVote();
    // rt.voteTest("127.0.0.1", 5001);

    getchar();


    return 0;
}