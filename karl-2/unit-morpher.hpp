#pragma once

#include "default-code.hpp"
#include "bwapi-precondition.hpp"

UnitPrecondition* morphUnit(UnitPrecondition* unit, ResourcesPrecondition* res, SupplyPrecondition* supply, const BWAPI::UnitType& ut, Precondition* extra = NULL);
UnitPrecondition* morphUnit(UnitPrecondition* unit, ResourcesPrecondition* res, const BWAPI::UnitType& ut, Precondition* extra = NULL);
UnitPrecondition* morphUnit(UnitPrecondition* unit, const BWAPI::UnitType& ut, Precondition* extra = NULL);
UnitPrecondition* morphUnit(const BWAPI::UnitType& ut, Precondition* extra = NULL);

struct UnitMorpherCode : public DefaultCode
{
	static void onMatchEnd();
    static void onTick();
	static void onDrawPlan();
};
