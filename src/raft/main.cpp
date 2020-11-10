#include <iostream>
#include "raft.h"


int main(int argc, char** argv) 
{
    Raft rt(1);
    rt.start();
    rt.peer("127.0.0.1", 5001);

    getchar();
    rt.voteTest("127.0.0.1", 5001);

    return 0;
}