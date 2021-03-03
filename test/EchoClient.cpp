#include "Log.h"
#include "Scheduler.h"
#include "TcpClient.h"
#include <atomic>
#include <unistd.h>
#include <unordered_map>

using namespace leo;

class EchoClient : Noncopyable {
public:
    EchoClient(Scheduler::ptr scheduler, const IpAddress& server_addr)
        : scheduler_(scheduler)
    {
        std::atomic_init(&quit_, false);
        tcpClient_ = std::make_shared<TcpClient>(server_addr);
    }
    void start()
    {
        scheduler_->addTask(std::bind(&EchoClient::handleConnection, this));
        scheduler_->runAfter(3 * Timestamp::kMicrosecondsPerSecond,
            std::make_shared<Coroutine>([&]() {
                quit_.store(true);
                LOG_INFO << "timeout";
            }));
    }

private:
    void handleConnection();
    void onDisconnect();
    bool isQuit()
    {
        return quit_.load();
    }

private:
    Scheduler::ptr scheduler_;
    TcpConnection::ptr tcpConnection_;
    TcpClient::ptr tcpClient_;
    std::atomic<bool> quit_;
};

void EchoClient::handleConnection()
{
    TcpConnection::ptr conn = tcpClient_->connect();
    conn->setTcpNoDelay(true);

    Buffer::ptr buffer = std::make_shared<Buffer>();
    std::string message = "evenleo";

    conn->write(message);

    while (!isQuit() && conn->read(buffer) > 0) {
        std::string str(buffer->peek(), buffer->readableBytes());
        std::cout << "send: " << str << std::endl;
        conn->write(buffer);
    }
    conn->shutdown();
    conn->readUntilZero();
    conn->close();
    scheduler_->stop();
}

int main(int argc, char** argv)
{
    Singleton<Logger>::instance()->addAppender("console", LogAppender::ptr(new ConsoleAppender()));
    if (argc < 2) 
        exit(0);
    IpAddress server_addr(argv[1], atoi(argv[2]));
    Scheduler::ptr scheduler = std::make_shared<Scheduler>(3);
    scheduler->startAsync();
    EchoClient client(scheduler, server_addr);
    client.start();
    scheduler->wait();
    sleep(5);

    return 0;
}
