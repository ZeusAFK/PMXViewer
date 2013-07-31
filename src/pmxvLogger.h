#ifndef PMXV_LOGGER_H
#define PMXV_LOGGER_H

#include <fstream>
#include <string>

class pmxvLogger
{
public:
	static pmxvLogger* get();
	
	~pmxvLogger();
	
	void e(const std::string &error);
	void d(const std::string &debug);
	void m(const std::string &message);
	
protected:
	pmxvLogger();
	pmxvLogger(const std::string &);
	
	void startLog(const std::string &);

	std::ofstream* logfile;

};

#define err pmxvLogger::get()->e
#define dbg pmxvLogger::get()->d
#define log pmxvLogger::get()->m

#endif // PMXV_LOGGER_H

