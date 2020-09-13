#include "../serializer.h"
#include <iostream>

using namespace leo;
using namespace std;

struct Student {
    int age;
    std::string name;
    Student() {}
    Student(int a, const std::string& n) 
        : age(a), name(n) {}
    
    friend Serializer& operator>>(Serializer& in, Student& s)
    {
        in >> s.age >> s.name;
        return in;
    }
    friend Serializer& operator<<(Serializer& out, Student& s)
    {
        out << s.age << s.name;
        return out;
    }
};

int main(int argc, char** argv)
{
    Serializer sr;
    /* 基本类型 */
    int n = 24;
    int v;
    sr << n;  // 序列化
    sr >> v;  // 反序列化
    cout << v << endl;

    /* 自定义类型类型 */
    Student src(23, "evenleo");
    Student dest;
    sr << src;
    sr >> dest;
    cout << dest.name << ", " << dest.age << endl;

    return 0;
}