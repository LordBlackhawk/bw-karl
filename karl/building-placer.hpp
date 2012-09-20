#pragma once

#include "default-code.hpp"
#include "bwapi-precondition.hpp"

BuildingPositionPrecondition* getBuildingPosition(const BWAPI::UnitType& ut, const BWAPI::TilePosition& pos);
BuildingPositionPrecondition* getBuildingPosition(const BWAPI::UnitType& ut);

struct BuildingPlacerCode : public DefaultCode
{
	static void onMatchBegin();
};
