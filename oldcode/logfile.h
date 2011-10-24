#ifndef LOGFILE_h
#define LOGFILE_h

#include <iostream>
#include <fstream>

class LogFile
{
private:
	std::ofstream		out;
	std::streambuf*		sbuf;

	LogFile()
		: out("./bwapi-data/logs/mylogfile.txt"), sbuf(std::cerr.rdbuf())
	{
		std::cerr.rdbuf(out.rdbuf());
	}

	~LogFile()
	{
		// Umleitung unbedingt wieder aufheben!
		std::cerr.rdbuf(sbuf);
	}

public:
	static LogFile& getInstance()
	{
		static LogFile lf;
		return lf;
	}

	static void write(const char* txt)
	{
		getInstance();
		std::cerr << "Frame " << Broodwar->getFrameCount() << ": " << txt << std::endl;
	}
};

inline void log(const char* txt)
{
	Broodwar->printf(txt);
	LogFile::write(txt);
}

#endif