#include "opening-expert.hpp"
#include "expert-registrar.hpp"
#include "plan/broodwar-plan-items.hpp"
#include "utils/log.hpp"
#include <BWTA.h>

REGISTER_EXPERT(OpeningExpert)

namespace
{
    bool canBuildHere(BWAPI::TilePosition position, BWAPI::UnitType type)
    {
      //returns true if we can build this type of unit here. Takes into account reserved tiles.
      return BWAPI::Broodwar->canBuildHere(NULL, position, type);
    }

    bool buildable(int x, int y)
    {
      //returns true if this tile is currently buildable, takes into account units on tile
      if (!BWAPI::Broodwar->isBuildable(x,y)) return false;
      std::set<BWAPI::Unit*> units = BWAPI::Broodwar->getUnitsOnTile(x, y);
      for(std::set<BWAPI::Unit*>::iterator i = units.begin(); i != units.end(); i++)
        if ((*i)->getType().isBuilding() && !(*i)->isLifted())
          return false;
      return true;
    }

    bool canBuildHereWithSpace(BWAPI::TilePosition position, BWAPI::UnitType type, int buildDist)
    {
      //returns true if we can build this type of unit here with the specified amount of space.
      //space value is stored in this->buildDistance.

      //if we can't build here, we of course can't build here with space
      if (!canBuildHere(position, type))
        return false;

      int width=type.tileWidth();
      int height=type.tileHeight();

      //make sure we leave space for add-ons. These types of units can have addons:
      if (type==BWAPI::UnitTypes::Terran_Command_Center ||
        type==BWAPI::UnitTypes::Terran_Factory ||
        type==BWAPI::UnitTypes::Terran_Starport ||
        type==BWAPI::UnitTypes::Terran_Science_Facility)
      {
        width+=2;
      }
      int startx = position.x() - buildDist;
      if (startx<0) return false;
      int starty = position.y() - buildDist;
      if (starty<0) return false;
      int endx = position.x() + width + buildDist;
      if (endx>BWAPI::Broodwar->mapWidth()) return false;
      int endy = position.y() + height + buildDist;
      if (endy>BWAPI::Broodwar->mapHeight()) return false;

      if (!type.isRefinery())
      {
        for(int x = startx; x < endx; x++)
          for(int y = starty; y < endy; y++)
              if (!buildable(x, y))
                return false;
      }

      if (position.x()>3)
      {
        int startx2=startx-2;
        if (startx2<0) startx2=0;
        for(int x = startx2; x < startx; x++)
          for(int y = starty; y < endy; y++)
          {
            std::set<BWAPI::Unit*> units = BWAPI::Broodwar->getUnitsOnTile(x, y);
            for(std::set<BWAPI::Unit*>::iterator i = units.begin(); i != units.end(); i++)
            {
              if (!(*i)->isLifted())
              {
                BWAPI::UnitType type=(*i)->getType();
                if (type==BWAPI::UnitTypes::Terran_Command_Center ||
                  type==BWAPI::UnitTypes::Terran_Factory ||
                  type==BWAPI::UnitTypes::Terran_Starport ||
                  type==BWAPI::UnitTypes::Terran_Science_Facility)
                {
                  return false;
                }
              }
            }
          }
      }
      return true;
    }

    BWAPI::TilePosition getBuildLocationNear(BWAPI::TilePosition position, BWAPI::UnitType type, int buildDist)
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
      while (length < BWAPI::Broodwar->mapWidth()) //We'll ride the spiral to the end
      {
        //if we can build here, return this tile position
        if (x >= 0 && x < BWAPI::Broodwar->mapWidth() && y >= 0 && y < BWAPI::Broodwar->mapHeight())
          if (canBuildHereWithSpace(BWAPI::TilePosition(x, y), type, buildDist))
            return BWAPI::TilePosition(x, y);

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
          if (dx == 0)
          {
            dx = dy;
            dy = 0;
          }
          else
          {
            dy = -dx;
            dx = 0;
          }
        }
        //Spiral out. Keep going.
      }
      return BWAPI::TilePositions::None;
    }
}

OpeningExpert::OpeningExpert()
    : poolPos(BWAPI::TilePositions::Unknown)
{ }

void OpeningExpert::prepare()
{
    BWTA::BaseLocation* base = BWTA::getStartLocation(BWAPI::Broodwar->self());
    if (base == NULL)
        return;
    BWAPI::TilePosition startPos = base->getTilePosition();
    poolPos = getBuildLocationNear(startPos, BWAPI::UnitTypes::Zerg_Spawning_Pool, 1);
}

void OpeningExpert::tick(Blackboard* blackboard)
{
    if (poolPos == BWAPI::TilePositions::Unknown)
        return;

    blackboard->addItem(new BuildPlanItem(BWAPI::UnitTypes::Zerg_Spawning_Pool, poolPos));
    LOG << "Pool added.";

    blackboard->removeExpert(this);
    delete this;
}
