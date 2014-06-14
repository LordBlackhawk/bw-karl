#pragma once

#include "bwplan.h"
#include <BWAPI.h>

void displayResources(const BWResources& res)
{
	BWAPI::Broodwar->drawTextScreen(5, 0, "At game time %d:", res.getTime());
	int line = 1;
	for (auto it : BWAllResourceIndices())
		if ((res.get(it) != 0) || (it.isLockable() && (res.getExisting(it) != 0)))
	{
		if (it.isLockable()) {
			BWAPI::Broodwar->drawTextScreen(5, 16*line, "- %d/%d %s", res.get(it), res.getExisting(it), it.getName().c_str());
		} else {
			BWAPI::Broodwar->drawTextScreen(5, 16*line, "-   %d  %s", res.get(it), it.getName().c_str());
		}
		++line;
	}
}
