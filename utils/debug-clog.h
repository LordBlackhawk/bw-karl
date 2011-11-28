#pragma once

#include "debug.h"
#include <iostream>

void logInternal(const std::string& file, int line, const std::string& functionname, int level, const std::string& txt)
{
	std::clog << file << ":" << line << "(" << functionname << ", " << level << "): " << txt << "\n";
}