#pragma once

#include "default-code.hpp"
#include "bwapi-precondition.hpp"

SupplyPrecondition* getSupply(const BWAPI::Race& r, int s);
SupplyPrecondition* getSupply(const BWAPI::UnitType& ut);
UnitPrecondition* registerSupplyUnit(UnitPrecondition* unit);

namespace SupplyMode
{
    enum { None, Auto };
}
void setSupplyMode(const BWAPI::Race& r, int mode);

struct SupplyCode : public DefaultCode
{
    static void onMatchBegin();
    static void onMatchEnd();
    static void onTick();
    static void onCheckMemoryLeaks();
    static void onDrawPlan(HUDTextOutput& hud);
};
