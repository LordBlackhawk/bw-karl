#include "log.hpp"

#include <iostream>

namespace
{
	int loglevel = 1;
}

bool logDisplay(const std::string&, int, const std::string&, int level)
{
	return (level <= loglevel);
}

void logInternal(const std::string& /*file*/, int /*line*/, const std::string& /*functionname*/,
                 int level, const std::string& txt)
{
	std::clog
		<< ((BWAPI::Broodwar != NULL) ? BWAPI::Broodwar->getFrameCount() : -1)
        << ((level == Log::Warning) ? " WARNING" : "")
        << ": "
		<< txt << "\n"
	//	<< "\tin " << file << ":" << line << " (" << functionname << "), prio = " << level << ")\n"
		;
}

void LogCode::onReadParameter(int /*argc*/, const char* argv[], int& cur)
{
    std::string txt = argv[cur];

    if (txt.size() < 2)
        return;
    
    if (txt[0] != '-' || txt[1] != 'v')
        return;
    
    if (txt[2] != '0' && txt[2] != '1' && txt[2] != '2') {
        WARNING << "Unreadable verbose mode '" << txt << "', expect -v0, -v1, or -v2.";
        return;
    }
    
    loglevel = txt[2] - '0';
    ++cur;
}
