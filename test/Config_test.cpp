#include "Config.h"
#include "Log.h"

using namespace leo;

int main(int argc, char** argv)
{
    Singleton<Logger>::instance()->addAppender("console", LogAppender::ptr(new ConsoleAppender()));
    Singleton<Config>::instance()->setPath("../conf/config.conf");
    std::string ip;
    ip = Singleton<Config>::instance()->getString("reactor", "ip", ip);
    LOG_INFO << "ip=" << ip;
    return 0;
}