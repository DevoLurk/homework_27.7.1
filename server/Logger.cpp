#include "Logger.h"

Logger::Logger()
{
	file_io.open(logs_path, std::ios::app);

	if (file_io.is_open())
		is_file_active = true;
	else
		is_file_active = false;
}

Logger::~Logger()
{
	file_io.close();
}

void Logger::writeStr(const std::string& log)
{
	shared_mutex.lock();
	file_io << log << std::endl;
	shared_mutex.unlock();
}

void Logger::readStr(std::string& log)
{
	shared_mutex.lock_shared();
	if (!std::getline(file_io, log))
		log = "Eroor";
	shared_mutex.unlock_shared();
}

bool Logger::is_active()
{
	return is_file_active;
}
