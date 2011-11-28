#pragma once

#include "debug.h"
#include <BWAPI.h>
#include <iostream>

void logInternal(const std::string& /*file*/, int /*line*/, const std::string& /*functionname*/, int /*level*/, const std::string& txt)
{
	std::clog
		<< ((BWAPI::Broodwar != NULL) ? BWAPI::Broodwar->getFrameCount() : -1) << ": "
		<< txt << "\n"
	//	<< "\tin " << file << ":" << line << " (" << functionname << "), prio = " << level << ")\n"
		;
}