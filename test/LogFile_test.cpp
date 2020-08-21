#include "LogFile.h"

void test()
{
	leo::LogFile log_file("temfile");

	std::string log = "log log log log log";
	
	log_file.persist(log.c_str(), log.size());
	log_file.flush();
}

int main() {
	
	test();
	return 0;
}
