#include "worker-expert.hpp"
#include "expert-registrar.hpp"
#include "plan/broodwar-plan-items.hpp"
#include "utils/log.hpp"
#include <BWTA.h>

REGISTER_EXPERT(WorkerExpert)

bool WorkerExpert::tick(Blackboard* blackboard)
{
    // Code to test the exception handler:
    // Creates one log file at frame 10.
    /*
    if (blackboard->getLastUpdateTime() == 10)
        LOG << "Current time: " << currentBlackboard->getLastUpdateTime();
    */
    return BasicPortExpert::tick(blackboard);
}

void WorkerExpert::beginTraversal()
{
    timeHorizont = currentBlackboard->getLastUpdateTime() + 3000;
}

void WorkerExpert::visitProvideUnitPort(ProvideUnitPort* port)
{
    if (port->isConnected() || !port->getUnitType().isWorker())
        return;

    if (port->isOnDemand()) {
        providePorts.insert(port);
        return;
    }

    if (port->estimatedTime > timeHorizont)
        return;

    BWTA::BaseLocation* base = BWTA::getStartLocation(currentBlackboard->self());
    BWAPI::Unit* mineral = *base->getStaticMinerals().begin();
    auto newItem = new GatherMineralsPlanItem(mineral, port);
    currentBlackboard->addItem(newItem);
    providePorts.insert(&newItem->provideUnit);
    //LOG << currentBlackboard->getLastUpdateTime() << ": Added GatherMineralPlanItem.";
}

void WorkerExpert::visitRequireUnitPort(RequireUnitPort* port)
{
    if (port->isConnected() || !port->getUnitType().isWorker())
        return;

    if (providePorts.empty())
        return;

    auto it = providePorts.begin();
    port->connectTo(*it);
    providePorts.erase(it);
}

void WorkerExpert::endTraversal()
{
    providePorts.clear();
}
