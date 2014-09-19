#include "resource-expert.hpp"
#include "expert-registrar.hpp"
#include "plan/broodwar-ports.hpp"
#include "plan/broodwar-plan-items.hpp"

REGISTER_EXPERT(ResourceExpert)

void ResourceExpert::visitResourcePort(ResourcePort* port)
{
    resources.push_back(port);
}

void ResourceExpert::visitGatherResourcesPlanItem(GatherResourcesPlanItem* item)
{
    if (!item->isImpossible())
        worker.push_back(item);
}

void ResourceExpert::endTraversal()
{
    simulate();
    resources.clear();
    worker.clear();
}

void ResourceExpert::simulate()
{
    const double mineralFactor = 0.045;
    Time currentTime = currentBlackboard->getLastUpdateTime();
    double currentMinerals = currentBlackboard->getInformations()->currentMinerals;
    int activeMineralWorker = 0;

    auto itWorker = worker.begin();
    for (auto itResources : resources) {
        int neededMinerals = itResources->getMinerals();
        Time finishTime = INFINITE_TIME;
        while (true) {
            if (activeMineralWorker > 0) {
                finishTime = currentTime + Time((neededMinerals - currentMinerals) / (activeMineralWorker * mineralFactor));
            }
            if ((itWorker == worker.end()) || (finishTime <= (*itWorker)->estimatedStartTime))
                break;

            Time nextTime = (*itWorker)->estimatedStartTime;
            if (nextTime > currentTime) {
                currentMinerals += (nextTime - currentTime) * (activeMineralWorker * mineralFactor);
                currentTime = nextTime;
            }
            ++activeMineralWorker;
            ++itWorker;
        }

        itResources->estimatedTime = finishTime;
        currentMinerals -= neededMinerals;
    }
}
