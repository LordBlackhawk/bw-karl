#pragma once

#include "default-code.hpp"
#include "bwapi-precondition.hpp"

RequirementsPrecondition* getRequirements(const BWAPI::UnitType& t);
bool isRequirement(const BWAPI::UnitType& t);
UnitPrecondition* registerRequirement(UnitPrecondition* unit);

struct RequirementsCode : public DefaultCode
{
	static void onMatchBegin();
	static void onMatchEnd();
	static void onTick();
	static void onUnitDestroy(BWAPI::Unit* unit);
	static void onCheckMemoryLeaks();
};
