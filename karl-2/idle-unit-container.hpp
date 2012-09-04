#pragma once

#include "default-code.hpp"
#include "bwapi-precondition.hpp"

UnitPrecondition* getIdleUnit(const BWAPI::UnitType& ut);

struct IdleUnitContainerCode : public DefaultCode
{
	static void onMatchBegin();
	static void onMatchEnd();
	static void onUnitCreate(BWAPI::Unit* unit);
	static void onUnitDestroy(BWAPI::Unit* unit);
};
