#pragma once

#include "default-code.hpp"
#include "bwapi-precondition.hpp"

UnitPrecondition* morphUnit(UnitPrecondition* unit, ResourcesPrecondition* res, SupplyPrecondition* supply, const BWAPI::UnitType& ut, const char* debugname = 0);
UnitPrecondition* morphUnit(UnitPrecondition* unit, ResourcesPrecondition* res, const BWAPI::UnitType& ut, const char* debugname = 0);
UnitPrecondition* morphUnit(UnitPrecondition* unit, const BWAPI::UnitType& ut, const char* debugname = 0);
UnitPrecondition* morphUnit(const BWAPI::UnitType& ut, const char* debugname = 0);

struct UnitMorpherCode : public DefaultCode
{
	static void onMatchEnd();
    static void onTick();
};
