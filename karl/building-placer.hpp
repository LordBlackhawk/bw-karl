#pragma once

#include "default-code.hpp"
#include "bwapi-precondition.hpp"
#include "array2d.hpp"
#include <BWTA.h>

struct TileInformation
{
    bool buildable;
    bool reserved;
    bool subtiles[4][4];

    TileInformation();
    int space(int d);
};

extern int mapWidth;
extern int mapHeight;
extern Array2d<TileInformation> tileInformations;

BuildingPositionPrecondition* getBuildingPosition(const BWAPI::UnitType& ut, const BWAPI::TilePosition& pos);
BuildingPositionPrecondition* getBuildingPosition(const BWAPI::UnitType& ut);
BuildingPositionPrecondition* getBuildingPosition(const BWAPI::UnitType& ut, std::set<BuildingPositionPrecondition*>& places);
BuildingPositionPrecondition* getExpoPosition(const BWAPI::UnitType& ut, BWTA::BaseLocation* location);
BuildingPositionPrecondition* getNextExpo(const BWAPI::UnitType& ut);

struct BuildingPlacerCode : public DefaultCode
{
    static void onMatchBegin();
};
