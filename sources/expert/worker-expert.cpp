#include "worker-expert.hpp"
#include "expert-registrar.hpp"
#include "plan/broodwar-plan-items.hpp"
#include "plan/broodwar-boundary-items.hpp"
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

    auto mineral = findMineralForWorker(port);
    if (mineral != NULL) {
        //LOG << currentBlackboard->getLastUpdateTime() << ": Add GatherMineralPlanItem...";
        auto newItem = new GatherMineralsPlanItem(mineral, port);
        currentBlackboard->addItem(newItem);
        providePorts.insert(&newItem->provideUnit);
    } else {
        providePorts.insert(port);
    }
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

MineralBoundaryItem* WorkerExpert::findMineralForWorker(ProvideUnitPort* port)
{
    MineralBoundaryItem* result = NULL;
    double bestValue = 1e10;
    for (auto base : currentBlackboard->getInformations()->ownBaseLocations) {
        for (auto mineral : base->minerals) {
            double dis = port->getPosition().getDistance(BWAPI::Position(mineral->getTilePosition()));
            double value = dis;
            if (value < bestValue) {
                result = mineral;
                bestValue = value;
            }
        }
    }
    return result;
}
