#pragma once

#include "default-code.hpp"
#include "bwapi-precondition.hpp"
#include "array2d.hpp"
#include <BWTA.h>

struct TileInformation
{
    bool                            buildable;
    bool                            subtiles[4][4];

    BuildingPositionPrecondition*   pre;
    BWAPI::Unit*                    building;
    int                             energytime;
    void*                           energyobj;
    int                             creeptime;
    void*                           creepobj;

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

UnitPrecondition* registerRangeBuilding(UnitPrecondition* pre);

struct BuildingPlacerCode : public DefaultCode
{
    static void onMatchBegin();
    static void onMatchEnd();
    static void onTick();
    static void onUnitCreate(BWAPI::Unit* unit);
    static void onUnitDestroy(BWAPI::Unit* unit);
    static void onUnitMorph(BWAPI::Unit* unit);
    static void onUnitShow(BWAPI::Unit* unit);
    static void onDrawPlan(HUDTextOutput& hud);
    static void onCheckMemoryLeaks();
};
