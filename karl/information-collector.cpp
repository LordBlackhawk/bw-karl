#include "information-collector.hpp"
#include "container-helper.hpp"
#include "log.hpp"

using namespace BWAPI;
using namespace BWTA;

namespace
{
    bool isBaseScouted(BaseLocation* loc)
    {
        TilePosition tp = loc->getTilePosition();
        return Broodwar->isExplored(tp);
    }

    std::map<Player*, BaseLocation*>    playerStartBaseMap;
    std::map<BaseLocation*, Player*>    basePlayerMap;
}

std::set<BaseLocation*> getUnscoutedStartLocations()
{
    std::set<BaseLocation*> result = getStartLocations();
    Containers::remove_if(result, isBaseScouted);
    return result;
}

std::set<BaseLocation*> getUnscoutedBaseLocations()
{
    std::set<BaseLocation*> result = getBaseLocations();
    Containers::remove_if(result, isBaseScouted);
    return result;
}

BWTA::BaseLocation* getEnemyStartLocation()
{
    auto it = playerStartBaseMap.find(Broodwar->enemy());
    if (it == playerStartBaseMap.end())
        return NULL;
    return it->second;
}

void InformationCode::onMatchBegin()
{

}

void InformationCode::onMatchEnd()
{
    playerStartBaseMap.clear();
    basePlayerMap.clear();
}

void InformationCode::onTick()
{ }

void InformationCode::onUnitCreate(BWAPI::Unit* unit)
{
    onUnitShow(unit);
}

void InformationCode::onUnitDestroy(BWAPI::Unit* /*unit*/)
{
    // Remove destroyed bases!
}

void InformationCode::onUnitShow(BWAPI::Unit* unit)
{
    UnitType ut = unit->getType();
    if (!ut.isResourceDepot())
        return;

    BaseLocation* loc = getNearestBaseLocation(unit->getTilePosition());
    if (loc == NULL)
        return;

    Player* owner = unit->getPlayer();
    if (loc->isStartLocation() && (playerStartBaseMap.count(owner) == 0))
        playerStartBaseMap[owner] = loc;

    if (basePlayerMap.count(loc) == 0)
        basePlayerMap[loc] = owner;
}