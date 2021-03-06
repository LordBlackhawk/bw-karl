#pragma once

#include "default-code.hpp"
#include "bwapi-precondition.hpp"

ResourcesPrecondition* getResources(int m, int g);
ResourcesPrecondition* getResources(const BWAPI::UnitType& ut);
ResourcesPrecondition* getResources(const BWAPI::TechType& tt);
ResourcesPrecondition* getResources(const BWAPI::UpgradeType& gt);

void setGasOnDemand(bool value);
int estimateResourcesAt(int time);

struct ResourcesCode : public DefaultCode
{
    static void onMatchBegin();
    static void onMatchEnd();
    static void onTick();
    static void onDrawPlan(HUDTextOutput& hud);
};
