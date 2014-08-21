#include "zergling-rush-opening-expert.hpp"
#include "expert-registrar.hpp"
#include "plan/broodwar-boundary-items.hpp"

//REGISTER_EXPERT(ZerglingRushOpeningExpert)

void ZerglingRushOpeningExpert::beginTraversal()
{
    poolPlanned=false;
    poolReady=false;
}

void ZerglingRushOpeningExpert::visitProvideUnitPort(ProvideUnitPort* port)
{
    if(port->getUnitType() == BWAPI::UnitTypes::Zerg_Spawning_Pool)
    {
        poolPlanned=true;
    }
}

void ZerglingRushOpeningExpert::visitOwnUnitBoundaryItem(OwnUnitBoundaryItem* item)
{
    if(item->getUnitType() == BWAPI::UnitTypes::Zerg_Spawning_Pool)
    {
        poolReady=!item->getUnit()->isMorphing();
    }
}

void ZerglingRushOpeningExpert::endTraversal()
{
    if(!poolPlanned && !poolReady)
    {
        currentBlackboard->build(BWAPI::UnitTypes::Zerg_Spawning_Pool);
    }

    if(currentBlackboard->getInformations()->unusedLarvaCount>0)
    {
        if(currentBlackboard->getInformations()->workerCount<4)
        {
            currentBlackboard->morph(BWAPI::UnitTypes::Zerg_Drone);
        }
        else if(poolReady)
        {
            currentBlackboard->morph(BWAPI::UnitTypes::Zerg_Zergling);
        }
    }
}
