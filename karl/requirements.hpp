#pragma once

#include "default-code.hpp"
#include "bwapi-precondition.hpp"

RequirementsPrecondition* getRequirements(const BWAPI::UnitType& t);
RequirementsPrecondition* getRequirements(const BWAPI::TechType& tt);
RequirementsPrecondition* getRequirements(const BWAPI::UpgradeType& gt, int level = 1);
bool isRequirement(const BWAPI::UnitType& t);
UnitPrecondition* registerRequirement(UnitPrecondition* unit);

namespace RequirementsMode
{
	enum { None, Auto };
}
void setRequirementsMode(int m);

struct RequirementsCode : public DefaultCode
{
	static void onMatchBegin();
	static void onMatchEnd();
	static void onTick();
	static void onUnitDestroy(BWAPI::Unit* unit);
	static void onCheckMemoryLeaks();
};
