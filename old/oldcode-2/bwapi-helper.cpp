#include "bwapi-helper.hpp"
#include <BWTA.h>
#include <cmath>

using namespace BWAPI;

namespace
{
    BWAPI::UnitType getUnitTypeByNameInternal(const std::string& name)
    {
        for (auto it : UnitTypes::allUnitTypes())
            if (it.getName() == name)
                return it;
        return UnitTypes::Unknown;
    }
}

BWAPI::UnitType getUnitTypeByName(const std::string& name)
{
    UnitType ut = getUnitTypeByNameInternal(name);
    if (ut != UnitTypes::Unknown)
        return ut;
    ut = getUnitTypeByNameInternal("Terran " + name);
    if (ut != UnitTypes::Unknown)
        return ut;
    ut = getUnitTypeByNameInternal("Protoss " + name);
    if (ut != UnitTypes::Unknown)
        return ut;
    ut = getUnitTypeByNameInternal("Zerg " + name);
    if (ut != UnitTypes::Unknown)
        return ut;
    return UnitTypes::Unknown;
}

BWAPI::Unit* getNearest(const std::set<BWAPI::Unit*>& units, const BWAPI::Position& p)
{
    if (units.empty())
        return NULL;

    auto it = units.begin();
    auto itend = units.end();
    Unit* result = *it;
    double dis = p.getDistance((*it)->getPosition());
    ++it;

    while (it != itend)
    {
        double newdis = p.getDistance((*it)->getPosition());
        if (newdis < dis) {
            result = *it;
            dis    = newdis;
        }
        ++it;
    }

    return result;
}

int clockPosition(const BWAPI::Position& pos)
{
    int x = pos.x() - 16*Broodwar->mapWidth();
    int y = pos.y() - 16*Broodwar->mapHeight();
    
    double pi = 3.14159265;
    double w  = std::atan2(-(double)y, (double)x);
    
    return ((8 + (int)(12.0 * (pi - w) / (2*pi))) % 12) + 1;
}

int clockPosition(BWAPI::Player* player)
{
    BWTA::BaseLocation* loc = BWTA::getStartLocation(player);
    if (loc == NULL)
        return 0;
    
    return clockPosition(loc->getPosition());
}
