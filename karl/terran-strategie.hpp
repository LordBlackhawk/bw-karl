#pragma once

#include "default-code.hpp"

struct TerranStrategieCode : public DefaultCode
{
    static bool isApplyable();
	static void onMatchBegin();
	static void onMatchEnd();
	static void onTick();
    static void onBaseMinedOut(BWTA::BaseLocation* base);
};
