#pragma once

#include "informations/informations.h"

#include "utils/array2d.h"

#include <BWAPI.h>
#include <BWTA.h>

class BuildingPlacer
{
  public:
    static BuildingPlacer& instance()
	{
		return Singleton<BuildingPlacer>::instance();
	}
	
	void init()
	{
		reserved.resize(BWAPI::Broodwar->mapWidth(), BWAPI::Broodwar->mapHeight(), false);
	}
	
	void reserveTiles(const BWAPI::UnitType& ut, const BWAPI::TilePosition& tp)
	{
		reserveTiles(tp, ut.tileWidth(), ut.tileHeight());
	}
	
	void freeTiles(const BWAPI::UnitType& ut, const BWAPI::TilePosition& tp)
	{
		freeTiles(tp, ut.tileWidth(), ut.tileHeight());
	}
	
	BWAPI::TilePosition find(BWAPI::UnitType type) const
	{
		BaseLocationInfoPtr mainbase = InformationKeeper::instance().self()->getMainBaseLocation();
		if (mainbase != NULL)
			return getBuildLocationNear(mainbase->getTilePosition(), type);
		
		return getBuildLocation(type);
	}

    BuildingPlacer();
    bool canBuildHere(BWAPI::TilePosition position, BWAPI::UnitType type) const;
    bool canBuildHereWithSpace(BWAPI::TilePosition position, BWAPI::UnitType type) const;
    bool canBuildHereWithSpace(BWAPI::TilePosition position, BWAPI::UnitType type, int buildDist) const;
    BWAPI::TilePosition getBuildLocation(BWAPI::UnitType type) const;
    BWAPI::TilePosition getBuildLocationNear(BWAPI::TilePosition position,BWAPI::UnitType type) const;
    BWAPI::TilePosition getBuildLocationNear(BWAPI::TilePosition position,BWAPI::UnitType type, int buildDist) const;
    bool buildable(int x, int y) const;
	void reserveTiles(BWAPI::TilePosition position, int width, int height);
    void freeTiles(BWAPI::TilePosition position, int width, int height);
    void setBuildDistance(int distance);
    int getBuildDistance() const;
    bool isReserved(int x, int y) const;
  private:
    int buildDistance;
	Array2d<bool> reserved;
};
