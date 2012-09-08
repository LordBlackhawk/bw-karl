#pragma once

#include "default-code.hpp"
#include "bwapi-precondition.hpp"

UnitPrecondition* getLarva();
UnitPrecondition* registerHatchery(UnitPrecondition* hatch);

struct LarvaCode : public DefaultCode
{
	static void onMatchBegin();
	static void onMatchEnd();
	static void onTick();
	static bool onAssignUnit(BWAPI::Unit* unit);
	static void onUnitDestroy(BWAPI::Unit* unit);
	static void onDrawPlan();
	//static void onDebug();
};
