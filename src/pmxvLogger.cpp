#include "pmxvLogger.h"
#include <assert.h>
#include <iostream>

const std::string CONST_DEFAULT_LOGNAME = "log.txt";

pmxvLogger::pmxvLogger()
{
	startLog(CONST_DEFAULT_LOGNAME);
}

pmxvLogger::pmxvLogger(const std::string &logfilename)
{
	if (logfilename.length() > 0)
		startLog(logfilename);
	else
		startLog(CONST_DEFAULT_LOGNAME);
}

pmxvLogger* pmxvLogger::get()
{
	static pmxvLogger *instance = 0;
	
	if (!instance)
		instance = new pmxvLogger("log.txt");
		
	return instance;
}
	
pmxvLogger::~pmxvLogger()
{
	logfile->close();
	delete logfile;
}

void pmxvLogger::startLog(const std::string &logfilename)
{
	assert(logfilename.length() > 0);
	logfile = new std::ofstream();
	logfile->open(logfilename, std::ios::app | std::ios::out);
	assert(logfile->is_open());
}
	
void pmxvLogger::e(const std::string &error)
{
	static std::string msg;
	msg = "error: " + error + '\n';
	*logfile  << msg;
	logfile->flush();
	std::cerr << msg;
}

void pmxvLogger::d(const std::string &debug)
{
#ifdef DEBUG
	static std::string msg;
	msg = "debug: " + error + '\n';
	*logfile  << msg;
	logfile->flush();
	std::cout << msg;
#endif
}

void pmxvLogger::m(const std::string &message)
{
	static std::string msg;
	msg = message + '\n';
	*logfile  << msg;
	logfile->flush();
	std::cout << msg;
}
