#pragma once

#include "default-code.hpp"
#include "bwapi-precondition.hpp"

std::pair<UnitPrecondition*, UnitPrecondition*> trainUnit(UnitPrecondition* worker, ResourcesPrecondition* res, SupplyPrecondition* supply, const BWAPI::UnitType& ut, const std::string& debugname = "");
std::pair<UnitPrecondition*, UnitPrecondition*> trainUnit(UnitPrecondition* worker, ResourcesPrecondition* res, const BWAPI::UnitType& ut, const std::string& debugname = "");
std::pair<UnitPrecondition*, UnitPrecondition*> trainUnit(ResourcesPrecondition* res, const BWAPI::UnitType& ut, const std::string& debugname = "");
std::pair<UnitPrecondition*, UnitPrecondition*> trainUnit(const BWAPI::UnitType& ut, const std::string& debugname = "");

void trainUnitEx(const BWAPI::UnitType& ut, const std::string& debugname = "");
void trainWorker(const BWAPI::UnitType& ut, const std::string& debugname = "");

struct UnitTrainerCode : public DefaultCode
{
    static void onMatchEnd();
    static void onTick();
    static bool onAssignUnit(BWAPI::Unit* unit);
    static void onDrawPlan(HUDTextOutput& hud);
    static void onCheckMemoryLeaks();
};
