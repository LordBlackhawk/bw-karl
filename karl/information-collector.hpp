#pragma once

#include "default-code.hpp"
#include "bwapi-precondition.hpp"

std::set<BWTA::BaseLocation*> getUnscoutedStartLocations();
std::set<BWTA::BaseLocation*> getUnscoutedBaseLocations();
BWTA::BaseLocation* getEnemyStartLocation();

struct InformationCode : public DefaultCode
{
    static void onMatchBegin();
    static void onMatchEnd();
    static void onTick();
    static void onUnitCreate(BWAPI::Unit* unit);
    static void onUnitDestroy(BWAPI::Unit* unit);
    static void onUnitShow(BWAPI::Unit* unit);
    static void onDrawPlan(HUDTextOutput& hud);
    static void onUnitMorph(BWAPI::Unit* unit);
};
