#ifndef LOGGER
#define LOGGER

#include<string>
#include<shared_mutex>
#include<thread>
#include<fstream>

class Logger
{
private:
	std::fstream file_io;
	std::string logs_path = "logs/logs.txt";
	std::shared_mutex shared_mutex;
	bool is_file_active;
public:
	Logger();
	~Logger();
	void writeStr(const std::string& log);
	void readStr(std::string& log);
	bool is_active();
};

#endif // LOGGER

