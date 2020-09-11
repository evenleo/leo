#include "Config.h"
#include "Log.h"

using namespace leo;


int main(int argc, char** argv)
{
    Singleton<Logger>::getInstance()->addAppender("console", LogAppender::ptr(new ConsoleAppender()));
    Singleton<Config>::getInstance()->setPath("../conf/config.conf");
    std::string ip;
    ip = Singleton<Config>::getInstance()->getString("reactor", "ip", ip);
    LOG_INFO << "ip=" << ip;

    return 0;
}