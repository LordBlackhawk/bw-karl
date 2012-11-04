#pragma once

#include "default-code.hpp"
#include "bwapi-precondition.hpp"

#include <BWTA.h>

BuildingPositionPrecondition* getBuildingPosition(const BWAPI::UnitType& ut, const BWAPI::TilePosition& pos);
BuildingPositionPrecondition* getBuildingPosition(const BWAPI::UnitType& ut);
BuildingPositionPrecondition* getExpoPosition(const BWAPI::UnitType& ut, BWTA::BaseLocation* location);
BuildingPositionPrecondition* getNextExpo(const BWAPI::UnitType& ut);

struct BuildingPlacerCode : public DefaultCode
{
	static void onMatchBegin();
};
