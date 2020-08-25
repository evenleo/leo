#include "../serializer.h"
#include <iostream>

using namespace leo;
using namespace std;

int main(int argc, char** argv)
{
    Buffer::ptr buffer = std::make_shared<Buffer>();
    Serializer s(buffer);
    int b = 100;
    std::string str = "evenleo";
    s << b;
    s << str;
    int v = 0;
    std::string name;
    s >> v;
    s >> name;
    cout << v << endl;
    cout << name << endl;

    return 0;
}