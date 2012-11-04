#pragma once

#include <sstream>

bool logDisplay(const std::string&, int, const std::string&, int);
void logInternal(const std::string&, int, const std::string&, int, const std::string&);

class Log
{
	public:
		Log(const std::string& f, int l, const std::string& fn, int v = 0)
			: file(f), line(l), functionname(fn), level(v)
		{
			display = logDisplay(file, line, functionname, level);
		}

		~Log()
		{
			if (display)
				logInternal(file, line, functionname, level, stream.str());
		}

		template <class T>
		Log& operator << (const T& t)
		{
			if (display)
				stream << t;
			return *this;
		}

	private:
		bool display;
		std::string file;
		int line;
		std::string functionname;
		int level;
		std::stringstream stream;
};

#define LOG  Log(__FILE__, __LINE__, __func__)
#define LOG1 Log(__FILE__, __LINE__, __func__, 1)
#define LOG2 Log(__FILE__, __LINE__, __func__, 2)
#define LOG3 Log(__FILE__, __LINE__, __func__, 3)
#define LOG4 Log(__FILE__, __LINE__, __func__, 4)
#define LOG5 Log(__FILE__, __LINE__, __func__, 5)