#include "log.hpp"

#include <iostream>
#include <cmath>

namespace
{
	int loglevel = 1;
}

bool logDisplay(const std::string&, int, const std::string&, int level)
{
	return (level <= loglevel);
}

void logInternal(const std::string& file, int line, const std::string& functionname,
                 int level, const std::string& txt)
{
	std::clog
            << ((level == Log::Warning) ? " WARNING" : "")
            << ": "
            << txt << "\n"
            ;
    
    if (level == Log::Warning)
        std::clog
            << "\tin " << file << ":" << line << " (" << functionname << ")\n"
            ;
}
