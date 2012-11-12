// ToDo:
//  * Do not build into mineral line.

#include "building-placer.hpp"
#include "array2d.hpp"
#include "valuing.hpp"
#include <BWTA.h>
#include <limits>

using namespace BWAPI;

int mapWidth = 0;
int mapHeight = 0;
Array2d<TileInformation> tileInformations;

namespace
{
    // the following code is copyed from BWSAL (with minor changes):

    const int buildDistance = 1;
    //Array2d<bool> reserved;
    
    bool isReserved(int x, int y)
    {
        return (tileInformations[x][y].reserved);
    }

    bool canBuildHere(const TilePosition& position, const UnitType& type)
    {
        //returns true if we can build this type of unit here. Takes into account reserved tiles.
        if (!Broodwar->canBuildHere(NULL, position, type))
            return false;
        for(int x = position.x(); x < position.x() + type.tileWidth(); x++)
            for(int y = position.y(); y < position.y() + type.tileHeight(); y++)
                if (isReserved(x, y))
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
                    if (!buildable(x, y) || isReserved(x, y))
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
        int upperX = std::min(position.x() + width, mapWidth);
        int upperY = std::min(position.y() + height, mapHeight);
        for(int x=position.x(); x<upperX; ++x)
            for(int y=position.y(); y<upperY; ++y)
                tileInformations[x][y].reserved = value;
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

BuildingPositionPrecondition* getBuildingPosition(const BWAPI::UnitType& ut, std::set<BuildingPositionPrecondition*>& places)
{
    for (auto it : places)
        if (it->ut == ut) 
    {
        places.erase(it);
        return it;
    }
    return getBuildingPosition(ut);
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
        if (!it->isIsland())
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
    mapWidth  = Broodwar->mapWidth();
    mapHeight = Broodwar->mapHeight();
    tileInformations.resize(mapWidth, mapHeight);
    
    for (int x=0; x<mapWidth; ++x)
        for (int y=0; y<mapHeight; ++y)
    {
        TileInformation& info = tileInformations[x][y];

        info.buildable = Broodwar->isBuildable(x, y);
        info.reserved  = false;
        for (int k=0; k<4; ++k)
            for (int l=0; l<4; ++l)
                info.subtiles[k][l] = Broodwar->isWalkable(4*x+k, 4*y+l);
    }
    
    for (auto it : BWTA::getBaseLocations())
    {
        reserveTiles(it->getTilePosition(), UnitTypes::Zerg_Hatchery, true);
    }
}

TileInformation::TileInformation()
    : buildable(false), reserved(false)
{
    for (int k=0; k<4; ++k)
        for (int l=0; l<4; ++l)
            subtiles[k][l] = false;
}

namespace
{
    int spaceHelper(bool b1, bool b2, bool b3, bool b4)
    {
        if (!b1)
            return 0;
        if (!b2)
            return 8;
        if (!b3)
            return 16;
        if (!b4)
            return 24;
        return 32;
    }
}

int TileInformation::space(int d)
{
    bool px[4] = { subtiles[0][0] && subtiles[0][1] && subtiles[0][2] && subtiles[0][3],
                   subtiles[1][0] && subtiles[1][1] && subtiles[1][2] && subtiles[1][3],
                   subtiles[2][0] && subtiles[2][1] && subtiles[2][2] && subtiles[2][3],
                   subtiles[3][0] && subtiles[3][1] && subtiles[3][2] && subtiles[3][3] };

    bool py[4] = { subtiles[0][0] && subtiles[1][0] && subtiles[2][0] && subtiles[3][0],
                   subtiles[0][1] && subtiles[1][1] && subtiles[2][1] && subtiles[3][1],
                   subtiles[0][2] && subtiles[1][2] && subtiles[2][2] && subtiles[3][2],
                   subtiles[0][3] && subtiles[1][3] && subtiles[2][3] && subtiles[3][3] };

    switch (d)
    {
        case 0: return spaceHelper(px[3], px[2], px[1], px[0]);
        case 1: return spaceHelper(py[0], py[1], py[2], py[3]);
        case 2: return spaceHelper(px[0], px[1], px[2], px[3]);
        case 3: return spaceHelper(py[3], py[2], py[1], py[0]);
        default:
            return -1;
    }
}
