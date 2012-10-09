#pragma once

#include "default-code.hpp"
#include "bwapi-precondition.hpp"

std::pair<UnitPrecondition*, UnitPrecondition*> trainUnit(UnitPrecondition* worker, ResourcesPrecondition* res, SupplyPrecondition* supply, const BWAPI::UnitType& ut, Precondition* extra = NULL);
std::pair<UnitPrecondition*, UnitPrecondition*> trainUnit(UnitPrecondition* worker, ResourcesPrecondition* res, const BWAPI::UnitType& ut, Precondition* extra = NULL);
std::pair<UnitPrecondition*, UnitPrecondition*> trainUnit(ResourcesPrecondition* res, const BWAPI::UnitType& ut, Precondition* extra = NULL);
std::pair<UnitPrecondition*, UnitPrecondition*> trainUnit(const BWAPI::UnitType& ut, Precondition* extra = NULL);

void trainUnitEx(const BWAPI::UnitType& ut);
void trainWorker(const BWAPI::UnitType& ut);

struct UnitTrainerCode : public DefaultCode
{
	static void onMatchEnd();
	static void onTick();
	static bool onAssignUnit(BWAPI::Unit* unit);
	static void onDrawPlan();
	static void onCheckMemoryLeaks();
};
