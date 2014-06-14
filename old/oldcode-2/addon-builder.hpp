#pragma once

#include "default-code.hpp"
#include "bwapi-precondition.hpp"

std::pair<UnitPrecondition*, UnitPrecondition*> buildAddon(UnitPrecondition* worker, ResourcesPrecondition* res, const BWAPI::UnitType& ut, Precondition* extra = NULL);
std::pair<UnitPrecondition*, UnitPrecondition*> buildAddon(ResourcesPrecondition* res, const BWAPI::UnitType& ut, Precondition* extra = NULL);
std::pair<UnitPrecondition*, UnitPrecondition*> buildAddon(const BWAPI::UnitType& ut, Precondition* extra = NULL);

void buildAddonEx(const BWAPI::UnitType& ut);

struct AddonBuilderCode : public DefaultCode
{
    static void onMatchEnd();
    static void onTick();
    static bool onAssignUnit(BWAPI::Unit* unit);
    static void onDrawPlan(HUDTextOutput& hud);
    static void onCheckMemoryLeaks();
};
