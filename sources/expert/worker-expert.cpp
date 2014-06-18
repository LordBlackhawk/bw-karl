#include "worker-expert.hpp"
#include "plan/broodwar-plan-items.hpp"
#include "utils/log.hpp"
#include <BWTA.h>

void WorkerExpert::visitAbstractPlanItem(AbstractPlanItem* item)
{
    if (item->estimatedStartTime > currentBlackboard->getLastUpdateTime() + 3000)
        return;
    BasicPortExpert::visitAbstractPlanItem(item);
}

void WorkerExpert::visitProvideUnitPort(ProvideUnitPort* port)
{
    if (port->isConnected() || !port->getUnitType().isWorker())
        return;

    // Find mineral patch for worker:
    BWTA::BaseLocation* base = BWTA::getStartLocation(currentBlackboard->self());
    BWAPI::Unit* mineral = *base->getStaticMinerals().begin();

    currentBlackboard->addItem(new GatherMineralsPlanItem(mineral, port));
    LOG << currentBlackboard->getLastUpdateTime() << ": Added GatherMineralPlanItem.";
}