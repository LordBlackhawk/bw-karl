#pragma once

#include "default-code.hpp"
#include "bwapi-precondition.hpp"

UnitPrecondition* flyBuilding(UnitPrecondition* building, BuildingPositionPrecondition* pos);

struct BuildingFlyerCode : public DefaultCode
{
    static void onMatchEnd();
    static void onTick();
    static void onDrawPlan(HUDTextOutput& hud);
    static void onCheckMemoryLeaks();
};
