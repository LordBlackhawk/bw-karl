#pragma once

#include "default-code.hpp"
#include "bwapi-precondition.hpp"

std::pair<UnitPrecondition*, UnitPrecondition*> buildUnit(UnitPrecondition* worker, ResourcesPrecondition* res, BuildingPositionPrecondition* pos, const BWAPI::UnitType& ut, Precondition* extra = NULL);
std::pair<UnitPrecondition*, UnitPrecondition*> buildUnit(UnitPrecondition* worker, BuildingPositionPrecondition* pos, const BWAPI::UnitType& ut, Precondition* extra = NULL);
std::pair<UnitPrecondition*, UnitPrecondition*> buildUnit(BuildingPositionPrecondition* pos, const BWAPI::UnitType& ut, Precondition* extra = NULL);
std::pair<UnitPrecondition*, UnitPrecondition*> buildUnit(const BWAPI::UnitType& ut, Precondition* extra = NULL);

struct UnitBuilderCode : public DefaultCode
{
	static void onMatchEnd();
	static void onTick();
	static bool onAssignUnit(BWAPI::Unit* unit);
	static void onUnitMorph(BWAPI::Unit* unit);
	static void onDrawPlan();
	static void onCheckMemoryLeaks();
};
