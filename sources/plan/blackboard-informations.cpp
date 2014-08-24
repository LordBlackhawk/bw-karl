#include "blackboard-informations.hpp"
#include "broodwar-boundary-items.hpp"
#include <iostream>

BaseLocation::BaseLocation(BlackboardInformations* o)
    : origin(NULL), owner(o)
{ }

Time BaseLocation::lastSeenComplete() const
{
    auto tp = getTilePosition();
    int xBegin = tp.x(), yBegin = tp.y();
    int xEnd = xBegin + BWAPI::UnitTypes::Zerg_Hatchery.tileWidth(), yEnd = yBegin + BWAPI::UnitTypes::Zerg_Hatchery.tileHeight();

    Time result;
    for (int x=xBegin; x<xEnd; ++x)
        for (int y=yBegin; y<yEnd; ++y)
            result = std::max(result, owner->fields[x][y].lastSeen);
    return result;
}

bool BaseLocation::isCompleteExplored() const
{
    auto tp = getTilePosition();
    int xBegin = tp.x(), yBegin = tp.y();
    int xEnd = xBegin + BWAPI::UnitTypes::Zerg_Hatchery.tileWidth(), yEnd = yBegin + BWAPI::UnitTypes::Zerg_Hatchery.tileHeight();

    bool result;
    for (int x=xBegin; x<xEnd; ++x)
        for (int y=yBegin; y<yEnd; ++y)
            result = result && owner->fields[x][y].isExplored();
    return result;
}

BlackboardInformations::~BlackboardInformations()
{
    for (auto it : allBaseLocations)
        delete it;
}

void BlackboardInformations::prepare()
{
    self = BWAPI::Broodwar->self();
    neutral = BWAPI::Broodwar->neutral();

    int mapWidth  = BWAPI::Broodwar->mapWidth();
    int mapHeight = BWAPI::Broodwar->mapHeight();
    fields.resize(mapWidth, mapHeight);
    for (int x=0; x<mapWidth; ++x)
        for (int y=0; y<mapHeight; ++y)
    {
        FieldInformations& field = fields[x][y];
        field.buildable = BWAPI::Broodwar->isBuildable(x, y);
        field.creep     = BWAPI::Broodwar->hasCreep(x, y);
    }

    auto mybase = BWTA::getStartLocation(self);
    for (auto base : BWTA::getBaseLocations()) {
        auto baselocation = new BaseLocation(this);
        baselocation->origin = base;
        for (auto unit : base->getMinerals())
            baselocation->minerals.insert(new ResourceBoundaryItem(unit, unit->getType(), this, baselocation));
        allBaseLocations.insert(baselocation);
        if (base == mybase)
            ownBaseLocations.insert(baselocation);
    }
}

void BlackboardInformations::fieldSeen(const BWAPI::TilePosition& tp, bool creep)
{
    FieldInformations& field = fields[tp.x()][tp.y()];
    field.lastSeen = lastUpdateTime;
    if (field.creep != creep) {
        field.creep = creep;
        if ((field.blocker != NULL) && field.blocker->getOwner()->isPlanItem())
            field.blocker->disconnect();
    }
}

void BlackboardInformations::printFieldInformations(std::ostream& stream)
{
    int mapWidth  = fields.getWidth();
    int mapHeight = fields.getHeight();
    stream << "\n";
    for (int y=0; y<mapHeight; ++y) {
        for (int x=0; x<mapWidth; ++x) {
            FieldInformations& field = fields[x][y];
            char c = '.';
            if (!field.buildable) {
                c = '#';
            } else if (field.blocker != NULL) {
                c = 'x';
            } else if (field.creep) {
                c = '~';
            }
            stream << c;
        }
        stream << "\n";
    }
}
