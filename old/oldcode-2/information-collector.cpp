#include "information-collector.hpp"
#include "container-helper.hpp"
#include "dual-graph.hpp"
#include "log.hpp"

using namespace BWAPI;
using namespace BWTA;

namespace
{
    struct UnitTypeInformation
    {
        UnitTypeInformation()
            : built(0), morphed(0), lost(0)
        { }
        int built;
        int morphed;
        int lost;
        int alive()
        {
            return built - morphed - lost;
        }
        void incBuilt()
        {
            ++built;
        }
        void incLost()
        {
            ++lost;
        }
        void incMorphed()
        {
            ++morphed;
        }
    };
    std::map<UnitType, UnitTypeInformation>     unitTypeMap;

    struct EnemyUnitInformation
    {
        Unit* unit;
        UnitType ut;
        TilePosition pos;
        EnemyUnitInformation(Unit* u)
            : unit(u), ut(unit->getType()), pos(unit->getTilePosition())
        {
            unitTypeMap[ut].incBuilt();
            if (ut.isBuilding())
                addUnitToGraph(ut, pos);
        }
        ~EnemyUnitInformation()
        {
            unitTypeMap[ut].incLost();
            if (ut.isBuilding())
                removeUnitFromGraph(ut, pos);
        }
        void show()
        {
            if (ut != unit->getType()) {
                morph();
            }
            if (ut.isBuilding() && (pos != unit->getTilePosition())) {
                removeUnitFromGraph(ut, pos);
                pos = unit->getTilePosition();
                addUnitToGraph(ut, pos);
            }
        }
        void morph()
        {
            if (ut.isBuilding())
                removeUnitFromGraph(ut, pos);
            unitTypeMap[ut].incMorphed();
            pos = unit->getTilePosition();
            ut = unit->getType();
            unitTypeMap[ut].incBuilt();
            if (ut.isBuilding())
                addUnitToGraph(ut, pos);
        }
    };
    std::map<Unit*, EnemyUnitInformation*>      cachedEnemyUnits;

    EnemyUnitInformation* lookupEnemyUnit(Unit* unit)
    {
        auto it = cachedEnemyUnits.find(unit);
        if (it != cachedEnemyUnits.end())
            return it->second;
        EnemyUnitInformation* result = new EnemyUnitInformation(unit);
        cachedEnemyUnits[unit] = result;
        return result;
    }
    
    void removeEnemyUnit(Unit* unit)
    {
        auto it = cachedEnemyUnits.find(unit);
        if (it == cachedEnemyUnits.end())
            return;
        delete it->second;
        cachedEnemyUnits.erase(it);
    }

    bool isBaseScouted(BaseLocation* loc)
    {
        TilePosition tp = loc->getTilePosition();
        return Broodwar->isExplored(tp);
    }

    std::map<Player*, BaseLocation*>    playerStartBaseMap;
    std::map<BaseLocation*, Player*>    basePlayerMap;
    
    void handleBases(Unit* unit)
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
    for (auto unit : Broodwar->getAllUnits())
        lookupEnemyUnit(unit)->show();
}

void InformationCode::onMatchEnd()
{
    playerStartBaseMap.clear();
    basePlayerMap.clear();
    for (auto it : cachedEnemyUnits)
        delete it.second;
    cachedEnemyUnits.clear();
    unitTypeMap.clear();
}

void InformationCode::onTick()
{ }

void InformationCode::onUnitCreate(BWAPI::Unit* unit)
{
    handleBases(unit);
    lookupEnemyUnit(unit);
}

void InformationCode::onUnitDestroy(BWAPI::Unit* unit)
{
    // Remove destroyed bases!
    removeEnemyUnit(unit);
}

void InformationCode::onUnitShow(BWAPI::Unit* unit)
{
    handleBases(unit);
    lookupEnemyUnit(unit)->show();
}

void InformationCode::onUnitMorph(BWAPI::Unit* unit)
{
    lookupEnemyUnit(unit)->morph();
}

namespace
{
    bool showEnemyStats = false;
}

void InformationCode::onSendText(const std::string& text)
{
    if (text == "/show enemystats")
        showEnemyStats = !showEnemyStats;
}

void InformationCode::onDrawPlan(HUDTextOutput& /*hud*/)
{
    if (!showEnemyStats) return;

    const int FIRST_LINE = 60;
    const int LINE_SEP   = 16;
    const int FIRST_COL  = 420;
    const int SECOND_COL = FIRST_COL + 150;
    const int THIRD_COL  = SECOND_COL + 20;
    const int FOURTH_COL = THIRD_COL  + 20;

    Broodwar->drawTextScreen(SECOND_COL, FIRST_LINE, "#");
    Broodwar->drawTextScreen(THIRD_COL,  FIRST_LINE, "m");
    Broodwar->drawTextScreen(FOURTH_COL, FIRST_LINE, "X");
    int line = 1;
    int minerals = 0, gas = 0;
    for (auto it : unitTypeMap) {
        Broodwar->drawTextScreen(FIRST_COL,  FIRST_LINE + LINE_SEP * line, "%s", it.first.c_str());
        Broodwar->drawTextScreen(SECOND_COL, FIRST_LINE + LINE_SEP * line, "%d", it.second.built);
        Broodwar->drawTextScreen(THIRD_COL,  FIRST_LINE + LINE_SEP * line, "%d", it.second.morphed);
        Broodwar->drawTextScreen(FOURTH_COL, FIRST_LINE + LINE_SEP * line, "%d", it.second.lost);
        ++line;

        if ((it.first != UnitTypes::Zerg_Larva) && (it.first != UnitTypes::Zerg_Egg)) {
            minerals += it.first.mineralPrice() * it.second.built;
            gas      += it.first.gasPrice() * it.second.built;
        }
    }

    Player* self = Broodwar->self();
    Broodwar->drawTextScreen(FIRST_COL, FIRST_LINE + LINE_SEP * (line+1), "Sum enemy resources: %d // %d", minerals, gas);
    Broodwar->drawTextScreen(FIRST_COL, FIRST_LINE + LINE_SEP * (line+2), "Sum own   resources: %d // %d", self->gatheredMinerals() + 600, self->gatheredGas());
}