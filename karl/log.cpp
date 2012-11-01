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

std::ostream& operator << (std::ostream& stream, const BWAPI::Position& pos)
{
    return stream << "(" << pos.x() << ", " << pos.y() << ")";
}

std::ostream& operator << (std::ostream& stream, const BWAPI::TilePosition& tp)
{
    return stream << "TP(" << tp.x() << ", " << tp.y() << ")";
}

std::ostream& operator << (std::ostream& stream, const BWAPI::Player*& player)
{
    return stream << player->getName();
}

std::ostream& operator << (std::ostream& stream, const BWAPI::UnitType& ut)
{
    return stream << ut.getName();
}

std::ostream& operator << (std::ostream& stream, const BWAPI::TechType& tt)
{
    return stream << tt.getName();
}

std::ostream& operator << (std::ostream& stream, const BWAPI::UpgradeType& gt)
{
    return stream << gt.getName();
}

std::ostream& operator << (std::ostream& stream, const BWAPI::Error& err)
{
    return stream << err.toString();
}

std::ostream& operator << (std::ostream& stream, const BWAPI::Race& race)
{
    return stream << race.getName();
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
