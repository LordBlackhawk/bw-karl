// ToDo:
//  * Do not build into mineral line.

#include "building-placer.hpp"
#include "array2d.hpp"
#include "valuing.hpp"
#include <BWTA.h>
#include <limits>

using namespace BWAPI;

namespace
{
	// the following code is copyed from BWSAL (with minor changes):

	const int buildDistance = 1;
	Array2d<bool> reserved;

	bool canBuildHere(const TilePosition& position, const UnitType& type)
	{
		//returns true if we can build this type of unit here. Takes into account reserved tiles.
		if (!Broodwar->canBuildHere(NULL, position, type))
			return false;
		for(int x = position.x(); x < position.x() + type.tileWidth(); x++)
			for(int y = position.y(); y < position.y() + type.tileHeight(); y++)
				if (reserved[x][y])
					return false;
		return true;
	}
	
	bool buildable(int x, int y)
	{
		//returns true if this tile is currently buildable, takes into account units on tile
		if (!Broodwar->isBuildable(x,y)) return false;
		for (auto i : BWAPI::Broodwar->getUnitsOnTile(x, y))
			if (i->getType().isBuilding() && !i->isLifted())
				return false;
		return true;
	}
	
	bool canBuildHereWithSpace(const TilePosition& position, const UnitType& type)
	{
		//returns true if we can build this type of unit here with the specified amount of space.
		//space value is stored in this->buildDistance.

		//if we can't build here, we of course can't build here with space
		if (!canBuildHere(position, type))
			return false;

		int width=type.tileWidth();
		int height=type.tileHeight();

		//make sure we leave space for add-ons. These types of units can have addons:
		if (type==UnitTypes::Terran_Command_Center ||
			type==UnitTypes::Terran_Factory || 
			type==UnitTypes::Terran_Starport ||
			type==UnitTypes::Terran_Science_Facility)
		{
			width+=2;
		}
		int startx = position.x() - buildDistance;
		if (startx<0) return false;
		int starty = position.y() - buildDistance;
		if (starty<0) return false;
		int endx = position.x() + width + buildDistance;
		if (endx>Broodwar->mapWidth()) return false;
		int endy = position.y() + height + buildDistance;
		if (endy>Broodwar->mapHeight()) return false;

		if (!type.isRefinery()) {
			for(int x = startx; x < endx; x++)
				for(int y = starty; y < endy; y++)
					if (!buildable(x, y) || reserved[x][y])
						return false;
		}

		if (position.x()>3) {
			int startx2=startx-2;
			if (startx2<0) startx2=0;
			for(int x = startx2; x < startx; x++)
				for(int y = starty; y < endy; y++)
			{
				for (auto i : Broodwar->getUnitsOnTile(x, y))
					if (!i->isLifted()) {
						UnitType type = i->getType();
						if (type==UnitTypes::Terran_Command_Center ||
							type==UnitTypes::Terran_Factory || 
							type==UnitTypes::Terran_Starport ||
							type==UnitTypes::Terran_Science_Facility)
						{
							return false;
						}
					}
			}
		}
		return true;
	}
	
	TilePosition getBuildLocationNear(const TilePosition& position, const UnitType& type)
	{
		//returns a valid build location near the specified tile position.
		//searches outward in a spiral.
		int x      = position.x();
		int y      = position.y();
		int length = 1;
		int j      = 0;
		bool first = true;
		int dx     = 0;
		int dy     = 1;
		while (length < Broodwar->mapWidth()) //We'll ride the spiral to the end
		{
			//if we can build here, return this tile position
			if (x >= 0 && x < Broodwar->mapWidth() && y >= 0 && y < Broodwar->mapHeight())
				if (canBuildHereWithSpace(TilePosition(x, y), type))
					return TilePosition(x, y);

			//otherwise, move to another position
			x = x + dx;
			y = y + dy;
			//count how many steps we take in this direction
			j++;
			if (j == length) //if we've reached the end, its time to turn
			{
				//reset step counter
				j = 0;

				//Spiral out. Keep going.
				if (!first)
					length++; //increment step counter if needed

				//first=true for every other turn so we spiral out at the right rate
				first =! first;

				//turn counter clockwise 90 degrees:
				if (dx == 0) {
					dx = dy;
					dy = 0;
				} else {
					dy = -dx;
					dx = 0;
				}
			}
			//Spiral out. Keep going.
		}
		return TilePositions::None;
	}
	
	void reserveTiles(const TilePosition& position, int width, int height, bool value)
	{
		for(int x = position.x(); x < position.x() + width && x < reserved.getWidth(); ++x)
			for(int y = position.y(); y < position.y() + height && y < reserved.getHeight(); ++y)
				reserved[x][y] = value;
	}
	
	void reserveTiles(const TilePosition& position, const UnitType& type, bool value)
	{
		int width=type.tileWidth();
		int height=type.tileHeight();

		//make sure we leave space for add-ons. These types of units can have addons:
		if (type==UnitTypes::Terran_Command_Center ||
			type==UnitTypes::Terran_Factory || 
			type==UnitTypes::Terran_Starport ||
			type==UnitTypes::Terran_Science_Facility)
		{
			width+=2;
		}
		
		reserveTiles(position, width, height, value);
	}
	
	struct BuildingPositionInternal : public BuildingPositionPrecondition
	{
		BuildingPositionInternal(const BWAPI::UnitType& t, const BWAPI::TilePosition& p)
			: BuildingPositionPrecondition(t, p)
		{
			reserveTiles(pos, ut, true);
		}
		
		~BuildingPositionInternal()
		{
			reserveTiles(pos, ut, false);
		}
	};
}

BuildingPositionPrecondition* getBuildingPosition(const BWAPI::UnitType& ut, const BWAPI::TilePosition& pos)
{
	if (pos == TilePositions::None)
		return NULL;

	return new BuildingPositionInternal(ut, pos);
}

BuildingPositionPrecondition* getBuildingPosition(const BWAPI::UnitType& ut)
{
	TilePosition base = BWTA::getStartLocation(Broodwar->self())->getTilePosition();
	TilePosition pos  = getBuildLocationNear(base, ut);
	return getBuildingPosition(ut, pos);
}

BuildingPositionPrecondition* getExpoPosition(const BWAPI::UnitType& ut, BWTA::BaseLocation* location)
{
    if (location == NULL)
        return NULL;

    return getBuildingPosition(ut, location->getTilePosition());
}

BuildingPositionPrecondition* getNextExpo(const BWAPI::UnitType& ut)
{
    BWTA::BaseLocation* home = BWTA::getStartLocation(Broodwar->self());
    BWTA::BaseLocation* bestlocation = NULL;
    ctype               bestvalue = std::numeric_limits<ctype>::min();
    
    for (auto it : BWTA::getBaseLocations())
        if (Broodwar->canBuildHere(NULL, it->getTilePosition(), ut, false))
    {
        ctype newvalue = valueExpo(it, home);
        if (newvalue > bestvalue) {
            bestlocation = it;
            bestvalue    = newvalue;
        }
    }

    return getExpoPosition(ut, bestlocation);
}

void BuildingPlacerCode::onMatchBegin()
{
	reserved.resize(Broodwar->mapWidth(), Broodwar->mapHeight(), false);
    
    for (auto it : BWTA::getBaseLocations())
    {
        reserveTiles(it->getTilePosition(), UnitTypes::Zerg_Hatchery, true);
    }
}
