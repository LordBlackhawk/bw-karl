#pragma once

#include "bwapi-precondition.hpp"
#include "default-code.hpp"
#include <BWTA.h>
#include <set>

std::set<BuildingPositionPrecondition*> designWallIn(BWTA::Region* region, BWTA::Chokepoint* cpoint);
std::set<BuildingPositionPrecondition*> designWallIn();

struct WallInCode : public DefaultCode
{
    static void onDrawPlan(HUDTextOutput& hud);
};
