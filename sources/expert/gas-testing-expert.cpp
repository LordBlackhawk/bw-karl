#include "gas-testing-expert.hpp"
#include "expert-registrar.hpp"

#include "plan/broodwar-boundary-items.hpp"
#include "plan/broodwar-plan-items.hpp"
#include "plan/broodwar-ports.hpp"

#include "utils/log.hpp"

REGISTER_EXPERT(GasTestingExpert)

bool GasTestingExpert::isApplicable(Blackboard* /*blackboard*/)
{
    return BWAPI::Broodwar->self()->getRace() == BWAPI::Races::Zerg;
}

/*
bool GasTestingExpert::tick(Blackboard* blackboard)
{
    auto extractor = blackboard->build(BWAPI::UnitTypes::Zerg_Extractor,);
    extractor->
    currentBlackboard->addItem(new GatherResourcesPlanItem(extractor, &blackboard->morph(BWAPI::UnitTypes::Zerg_Drone)->provideUnit));
    return false;
}
*/

void GasTestingExpert::beginTraversal()
{
    needGas=false;
    haveGas=false;
}

void GasTestingExpert::endTraversal()
{
    if(needGas && !haveGas)
    {
        LOG << "GasTestingExpert: need gas and have none -> build refinery!";
        currentBlackboard->build(currentBlackboard->self()->getRace().getRefinery());
    }
}

void GasTestingExpert::visitResourcePort(ResourcePort *port)
{
    if(port->isRequirePort() && port->getGas()>0)
        needGas=true;
}

void GasTestingExpert::visitProvideUnitExistancePort(ProvideUnitExistancePort* port)
{
    if(port->getUnitType().isRefinery())
        haveGas=true;
}

void GasTestingExpert::visitBuildPlanItem(BuildPlanItem* item)
{
    if(item->getUnitType().isRefinery())
        haveGas=true;
}

void GasTestingExpert::visitProvideResourcePort(ProvideResourcePort *port)
{
    if(port->getUnit()->getType().isRefinery())
    {
        haveGas=true;
    }
}

void GasTestingExpert::visitResourceBoundaryItem(ResourceBoundaryItem* item)
{
    if(item->getUnitType().isRefinery() && item->getUnit()->getPlayer()==currentBlackboard->self())
    {
        haveGas=true;
        if(item->numberOfWorkers()<3)
        {
            LOG << "GasTestingExpert: found refinery with only "<<item->numberOfWorkers()<<" workers assigned, morphing another one.";
            currentBlackboard->addItem(new GatherResourcesPlanItem(item, &currentBlackboard->morph(BWAPI::UnitTypes::Zerg_Drone, {ResourceCategory::Economy})->provideUnit));
        }
    }
}