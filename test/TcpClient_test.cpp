#include "Log.h"
#include "TcpClient.h"
#include "Scheduler.h"

#include <atomic>
#include <unistd.h>
#include <stdio.h>

using namespace leo;

class Benchmark;

class Session : public Noncopyable {
public:
	Session(Benchmark* owner, Scheduler* scheduler, const IpAddress& server_addr)
			:owner_(owner),
			 scheduler_(scheduler),
	   		 client_(server_addr),
	   		 bytes_read_(0),
			 message_read_(0) {

			}
	void start() {
		scheduler_->addTask(std::bind(&Session::handleConnection, this));
	}

	int64_t byteRead() {
		return bytes_read_;
	}

	int64_t messageRead() {
		return message_read_;
	}
	
private:
	void handleConnection();

private:
	Benchmark* owner_;
	Scheduler* scheduler_;
	TcpClient client_;
	int64_t bytes_read_;
	int64_t message_read_;
};

class Benchmark : public Noncopyable {
public:
	Benchmark(Scheduler* scheduler,
			const IpAddress& server_addr,
			int block_size,
			int session_count,
			int timeout)
	: scheduler_(scheduler),
	  server_addr_(server_addr),
	  block_size_(block_size),
	  session_count_(session_count),
	  timeout_(timeout) {
		std::atomic_init(&quit_, false);
		std::atomic_init(&num_connected_, 0);

		for (int i = 0; i < block_size_; ++i) {
			message_.push_back(static_cast<char>(i % 128));
		}

		for (int i = 0; i < session_count_; ++i) {
			Session* session = new Session(this, scheduler_, server_addr_);
			sessions_.emplace_back(session);
			session->start();
		}

		scheduler_->runAfter(timeout * kMicrosecondsPerSecond, 
								std::make_shared<Coroutine>([&](){
											quit_.store(true);
											LOG_INFO << "timeout";
										}));
	}

	void onConnect() {
		num_connected_++;
	}

	void onDisconnect() {
		LOG_INFO << "onDisconnect, num_connected_=" << num_connected_.load();
		if (num_connected_.fetch_sub(1) == 1) {
			LOG_INFO << "all disconnected";	

			int64_t total_bytes_read = 0;
			int64_t total_messages_read = 0;
			for (const auto& session : sessions_) {
				total_bytes_read += session->byteRead();
				total_messages_read += session->messageRead();
			}
			LOG_INFO << total_bytes_read << " total bytes read"; 
			LOG_INFO << total_messages_read << " total messages read";
			LOG_INFO << static_cast<double>(total_bytes_read) / static_cast<double>(total_messages_read)
					<< " average message size";
			LOG_INFO << static_cast<double>(total_bytes_read) / (timeout_ * 1024 * 1024)
					<< " MiB/s throughput";

			scheduler_->stop();
		}
	}

	bool isQuit() {
		return quit_.load();
	}

	const std::string& message() const {
		return message_;
	}

private:
	Scheduler* scheduler_;
	IpAddress server_addr_;
	int block_size_;
	int session_count_;
	int timeout_;
	std::string message_;
	std::vector<std::unique_ptr<Session> > sessions_;
	std::atomic<bool> quit_;
	std::atomic<int> num_connected_;
};

void Session::handleConnection() {
	TcpConnection::ptr conn = client_.connect();
	conn->setTcpNoDelay(true);
	owner_->onConnect();

	Buffer::ptr buffer = std::make_shared<Buffer>();
	conn->write(owner_->message());
	while (!owner_->isQuit() && conn->read(buffer) > 0) {
		++message_read_;
		bytes_read_ += buffer->readableBytes();
		conn->write(buffer);
	}
	conn->shutdown();
	conn->readUntilZero();
	conn->close();
	owner_->onDisconnect();
}

int main(int argc, char* argv[]) {
	Singleton<Logger>::getInstance()->addAppender("console", LogAppender::ptr(new ConsoleAppender()));
	if (argc < 3) {
		LOG_ERROR << "please input session_count!";
	}
	IpAddress server_addr("127.0.0.1", 5000);
	int threads_num = atoi(argv[1]);
	int block_size = 40;
	int session_count = atoi(argv[2]);
	int timeout = 10;

	Scheduler scheduler(threads_num);
	scheduler.startAsync();
	
	Benchmark benchmark(&scheduler, server_addr, block_size, session_count, timeout);

	scheduler.wait();
	return 0;
}