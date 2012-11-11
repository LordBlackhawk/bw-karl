#include "bwapi-helper.hpp"

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
