#pragma once

#include "bwplan.h"
#include <BWAPI.h>

void displayResources(const BWResources& res)
{
	Broodwar->drawTextScreen(5, 0, "At game time %d, we have the following resources:", res.getTime());
	int line = 1;
	for (auto it : BWResourceIndices)
		if (res.get(it) != 0)
	{
		if (it.isLockable()) {
			Broodwar->drawTextScreen(5, 16*line, "- %d/%d %ss", res.get(it), res.getLocked(it), it.getUserName());
		} else {
			Broodwar->drawTextScreen(5, 16*line, "-   %d  %ss", res.get(it), it.getUserName());
		}
		++line;
	}
}
