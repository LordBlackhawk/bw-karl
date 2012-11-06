#include "arena.hpp"
#include "squad.hpp"
#include "unit-micromanagement.hpp"
#include "precondition-helper.hpp"
#include "log.hpp"
#include <BWTA.h>

using namespace BWAPI;
using namespace BWTA;
using namespace UnitTypes;

namespace
{
    Squad*  squad = NULL;
}

bool ArenaCode::isApplyable()
{
    Player* self = Broodwar->self();
    std::map<UnitType, int> units;
    for (auto it : self->getUnits())
        units[it->getType()] += 1;

    if ((units[Terran_Command_Center] > 0) || (units[Zerg_Hatchery] > 0) || (units[Protoss_Nexus] > 0))
        return false;

    if (getRegions().size() > 1)
        return false;

    return true;
}

void ArenaCode::onMatchBegin()
{
    LOG << "Playing arena...";

    squad = new Squad();
    squad->setName("arena");
    
    Player* self = Broodwar->self();
    for (auto it : self->getUnits())
        onAssignUnit(it);
    
    squad->defend(squad->getCenter());
}

void ArenaCode::onMatchEnd()
{
    release(squad);
}

void ArenaCode::onTick()
{

}

bool ArenaCode::onAssignUnit(BWAPI::Unit* unit)
{
    UnitType type = unit->getType();
    if ((type == Terran_Marine) || (type == Terran_Medic) || (type == Terran_Firebat)) {
        squad->addUnit(unit);
        return true;
    }
    return false;
}
