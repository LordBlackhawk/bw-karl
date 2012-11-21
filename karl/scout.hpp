#pragma once

#include "default-code.hpp"
#include "bwapi-precondition.hpp"

void useScout(UnitPrecondition* pre);

struct ScoutCode : public DefaultCode
{
    static void onMatchBegin();
    static void onMatchEnd();
    static void onTick();
};
