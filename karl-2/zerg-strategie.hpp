#pragma once

#include "default-code.hpp"

struct ZergStrategieCode : public DefaultCode
{
	static void onMatchBegin();
	static void onMatchEnd();
	static void onTick();
};
