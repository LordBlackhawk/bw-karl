// ToDo:
// - still missing units (geyser, enemy units).
// - still missing creep created while playing.
// - still missing flying buildings.
// - no unit tests yet.
// - implement movable correctly and not as buildable.

#include "building-placement-expert.hpp"
#include "expert-registrar.hpp"
#include "plan/broodwar-ports.hpp"
#include "plan/broodwar-plan-items.hpp"
#include "utils/log.hpp"

REGISTER_EXPERT(BuildingPlacementExpert)

void BuildingPlacementExpert::beginTraversal()
{
    mapWidth  = currentBlackboard->getInformations()->fields.getWidth();
    mapHeight = currentBlackboard->getInformations()->fields.getHeight();

    buildDistance = 1;

    auto& ownBaseLocations = currentBlackboard->getInformations()->ownBaseLocations;
    assert(ownBaseLocations.size() > 0);

    startPos = (*ownBaseLocations.begin())->getTilePosition();
    startPos = (*ownBaseLocations.begin())->getTilePosition();
}

void BuildingPlacementExpert::visitRequireSpacePort(RequireSpacePort* port)
{
    if (port->isConnected())
        return;

    unitType = port->getUnitType();
    BWAPI::TilePosition pos = getBuildLocationNear();
    if (pos != BWAPI::TilePositions::None) {
        LOG << "found position: " << pos.x() << ", " << pos.y();
        port->connectTo(pos);
    }
}

BWAPI::TilePosition BuildingPlacementExpert::getBuildLocationNear()
{
    //returns a valid build location near the specified tile position.
    //searches outward in a spiral.
    int x      = startPos.x();
    int y      = startPos.y();
    int length = 1;
    int j      = 0;
    bool first = true;
    int dx     = 0;
    int dy     = 1;
    while (length < mapWidth) { //We'll ride the spiral to the end
        //if we can build here, return this tile position
        if ((x >= 0) && (x < mapWidth) && (y >= 0) && (y < mapHeight))
            if (canBuildHereWithSpace(x, y))
                return BWAPI::TilePosition(x, y);

        //otherwise, move to another position
        x = x + dx;
        y = y + dy;
        //count how many steps we take in this direction
        j++;
        if (j == length) { //if we've reached the end, its time to turn
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
    return BWAPI::TilePositions::None;
}

bool BuildingPlacementExpert::canBuildHereWithSpace(int x, int y)
{
    //returns true if we can build this type of unit here with the specified amount of space.
    //space value is stored in this->buildDistance.

    //if we can't build here, we of course can't build here with space
    if (!canBuildHere(x, y))
        return false;

    int width = unitType.tileWidth();
    int height = unitType.tileHeight();

    //make sure we leave space for add-ons. These types of units can have addons:
    if (unitType.canBuildAddon())
        width += 2;

    int startx = x - buildDistance;
    if (startx < 0) return false;
    int starty = y - buildDistance;
    if (starty < 0) return false;
    int endx = x + width + buildDistance;
    if (endx > mapWidth) return false;
    int endy = y + height + buildDistance;
    if (endy > mapHeight) return false;

    for(int k=startx; k<endx; k++)
        for(int l=starty; l<endy; l++)
            if (!movable(k, l))
                return false;

    return true;
}

bool BuildingPlacementExpert::canBuildHere(int x, int y)
{
    int width = unitType.tileWidth();
    int height = unitType.tileHeight();

    if (unitType.canBuildAddon())
        width += 2;

    int startx = x;
    if (startx < 0) return false;
    int starty = y;
    if (starty < 0) return false;
    int endx = x + width;
    if (endx > mapWidth) return false;
    int endy = y + height;
    if (endy > mapHeight) return false;

    for(int k=startx; k<endx; k++)
        for(int l=starty; l<endy; l++)
            if (!buildable(k, l))
                return false;

    return true;
}

bool BuildingPlacementExpert::buildable(int x, int y)
{
    auto& field = currentBlackboard->getInformations()->fields[x][y];
    if (unitType.requiresCreep() && !field.creep)
        return false;
    return field.buildable && (field.blocker == NULL);
}

bool BuildingPlacementExpert::movable(int x, int y)
{
    return buildable(x, y);
}
