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

void WorkerExpert::visitProvideUnitPort(ProvideUnitPort* port)
{
    if (port->isConnected() || !port->getUnitType().isWorker())
        return;

    providePorts.insert(port);
}

void WorkerExpert::visitRequireUnitPort(RequireUnitPort* port)
{
    if (port->isConnected() || !port->getUnitType().isWorker())
        return;

    if (providePorts.empty()) {
        //LOG << "no provide ports yet.";
        return;
    }

    auto it = providePorts.begin();
    port->connectTo(*it);
    providePorts.erase(it);
}

void WorkerExpert::endTraversal()
{
    Time timeHorizont = currentBlackboard->getLastUpdateTime() + 3000;
    for (auto port : providePorts) {
        if ((port->estimatedTime > timeHorizont) || port->isOnDemand())
            continue;

        BWTA::BaseLocation* base = BWTA::getStartLocation(currentBlackboard->self());
        BWAPI::Unit* mineral = *base->getStaticMinerals().begin();

        currentBlackboard->addItem(new GatherMineralsPlanItem(mineral, port));
        //LOG << currentBlackboard->getLastUpdateTime() << ": Added GatherMineralPlanItem.";
    }
    providePorts.clear();
}
