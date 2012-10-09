#pragma once

#include "default-code.hpp"
#include "bwapi-precondition.hpp"

ResourcesPrecondition* getResources(int m, int g);
ResourcesPrecondition* getResources(const BWAPI::UnitType& ut);

struct ResourcesCode : public DefaultCode
{
	static void onMatchBegin();
	static void onMatchEnd();
	static void onTick();
};
