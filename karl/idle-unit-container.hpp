#pragma once

#include "default-code.hpp"
#include "bwapi-precondition.hpp"

UnitPrecondition* getIdleUnit(const BWAPI::UnitType& ut);
void rememberIdle(UnitPrecondition* unit);
UnitPrecondition* rememberFirst(const std::pair<UnitPrecondition*, UnitPrecondition*>& unit);
UnitPrecondition* rememberSecond(const std::pair<UnitPrecondition*, UnitPrecondition*>& unit);

UnitPrecondition* createUnit(const BWAPI::UnitType& ut);

int nextUnitAvaiable(const BWAPI::UnitType& ut);

struct IdleUnitContainerCode : public DefaultCode
{
	static void onMatchEnd();
	static void onTick();
	static void onUnitCreate(BWAPI::Unit* unit);
	static void onUnitDestroy(BWAPI::Unit* unit);
};
