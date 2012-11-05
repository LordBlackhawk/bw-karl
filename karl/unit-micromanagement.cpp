
// ToDo:
// *

#include "log.hpp"

#include "unit-micromanagement.hpp"
#include "squad.hpp"

#include <BWTA.h>

using namespace BWAPI;

namespace
{
    std::set<UnitMicromanagement*> units;

}
/////////////////////////////////////


void UnitMicromanagementCode::onMatchBegin()
{
    
}

void UnitMicromanagementCode::onMatchEnd()
{
    for(auto it:units)
    {
        delete it;
    }
    units.clear();
}

void UnitMicromanagementCode::onTick()
{
    for(auto it:units)
    {
        it->onTick();
    }
}

void UnitMicromanagementCode::onDrawPlan()
{
    for(auto it:units)
    {
        it->onDrawPlan();
    }
}

void UnitMicromanagementCode::onCheckMemoryLeaks()
{
    
}

void UnitMicromanagementCode::onUnitCreate(BWAPI::Unit* unit)
{
    if (unit->getPlayer() != Broodwar->self())
        return;
    
    if(unit->getClientInfo()!=NULL)
    {
        WARNING << "UnitMicromanagement::onUnitCreate: unit "<<unit->getType().getName()<<" already has ClientInfo attached!?";
        return;
    }


#define UNIT_WRAPPER(TYPE,CLASS) \
    if(unit->getType()==TYPE) \
    { \
        DEBUG << "UnitMicromanagement::onUnitCreate: micromanagement attached to "<<unit->getType().getName(); \
        UnitMicromanagement *m=new CLASS(unit); \
        units.insert(m); \
        unit->setClientInfo(m); \
    } 
    
    UNIT_WRAPPER(UnitTypes::Terran_Marine,UnitMicromanagementMarine)
    UNIT_WRAPPER(UnitTypes::Terran_Medic,UnitMicromanagementMedic)
    UNIT_WRAPPER(UnitTypes::Terran_Firebat,UnitMicromanagementFirebat)
}

void UnitMicromanagementCode::onUnitDestroy(BWAPI::Unit*)
{
    //ignore for now as we remember all units till the end of the match...
}




//////////////////////////////////////


void UnitMicromanagement::_assignToSquad(Squad* squad)
{
    this->squad=squad;
}


void UnitMicromanagement::attack(BWAPI::Position pos)
{
    unit->attack(pos);
}

void UnitMicromanagement::defend(BWAPI::Position pos)
{
    unit->attack(pos);
}

bool UnitMicromanagement::isIdle()
{
    return unit->isIdle();
}



//////////////////////////////////////

void UnitMicromanagementMarine::onTick()
{
    
}

void UnitMicromanagementMarine::onDrawPlan()
{
    Broodwar->drawBoxMap(unit->getLeft(),unit->getTop(),unit->getRight(),unit->getBottom(),Colors::Teal,false);
}


//////////////////////////////////////

void UnitMicromanagementMedic::onTick()
{
    
}

void UnitMicromanagementMedic::onDrawPlan()
{
    Broodwar->drawBoxMap(unit->getLeft(),unit->getTop(),unit->getRight(),unit->getBottom(),Colors::Teal,false);
}


//////////////////////////////////////


void UnitMicromanagementFirebat::onTick()
{
    
}

void UnitMicromanagementFirebat::onDrawPlan()
{
    Broodwar->drawBoxMap(unit->getLeft(),unit->getTop(),unit->getRight(),unit->getBottom(),Colors::Teal,false);
}


