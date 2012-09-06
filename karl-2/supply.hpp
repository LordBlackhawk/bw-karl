#pragma once

#include "default-code.hpp"
#include "bwapi-precondition.hpp"

SupplyPrecondition* getSupply(const BWAPI::Race& r, int s);
SupplyPrecondition* getSupply(const BWAPI::UnitType& ut);
UnitPrecondition* registerSupplyUnit(UnitPrecondition* unit);

struct SupplyCode : public DefaultCode
{
	static void onMatchBegin();
	static void onMatchEnd();
    static void onTick();
	static void onDebug();
};
