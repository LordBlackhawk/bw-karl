#pragma once

#include "default-code.hpp"

struct ZergStrategieCode : public DefaultCode
{
    static bool isApplyable();
    static void onMatchBegin();
    static void onMatchEnd();
    static void onTick();
    static void onDrawPlan(HUDTextOutput& hud);
    static void onBaseMinedOut(BWTA::BaseLocation* base);
};
