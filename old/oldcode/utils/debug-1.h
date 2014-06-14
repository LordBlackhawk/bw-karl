#pragma once

#include "debug.h"

bool logDisplay(const std::string&, int, const std::string&, int level)
{
	return (level <= 1);
}