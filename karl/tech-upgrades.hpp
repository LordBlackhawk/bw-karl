#pragma once

#include "default-code.hpp"
#include "bwapi-precondition.hpp"

UnitPrecondition* researchTech(UnitPrecondition* worker, ResourcesPrecondition* res, const BWAPI::TechType& tt, Precondition* extra = NULL);
UnitPrecondition* researchTech(UnitPrecondition* worker, const BWAPI::TechType& tt, Precondition* extra = NULL);
UnitPrecondition* researchTech(const BWAPI::TechType& tt, Precondition* extra = NULL);

UnitPrecondition* upgradeTech(UnitPrecondition* worker, ResourcesPrecondition* res, const BWAPI::UpgradeType& gt, Precondition* extra = NULL);
UnitPrecondition* upgradeTech(UnitPrecondition* worker, const BWAPI::UpgradeType& gt, Precondition* extra = NULL);
UnitPrecondition* upgradeTech(const BWAPI::UpgradeType& gt, Precondition* extra = NULL);

void researchTechEx(const BWAPI::TechType& tt);
void upgradeTechEx(const BWAPI::UpgradeType& gt);

struct TechCode : public DefaultCode
{
	static void onMatchEnd();
	static void onTick();
	static void onDrawPlan(HUDTextOutput& hud);
	static void onCheckMemoryLeaks();
};
