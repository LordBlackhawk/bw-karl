#include "resource-expert.hpp"
#include "expert-registrar.hpp"
#include "plan/broodwar-ports.hpp"
#include "plan/broodwar-plan-items.hpp"

REGISTER_EXPERT(ResourceExpert)

void ResourceExpert::visitResourcePort(ResourcePort* port)
{
    resources.push_back(port);
}

void ResourceExpert::visitGatherMineralPlanItem(GatherMineralsPlanItem* item)
{
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
        auto calcTime = [&] () {
                if (activeMineralWorker == 0)
                    return INFINITE_TIME;
                return Time((neededMinerals - currentMinerals) / (activeMineralWorker * mineralFactor));
            };
    
        Time finishTime = calcTime();
        while ((itWorker != worker.end()) && (finishTime > (*itWorker)->estimatedStartTime)) {
            Time nextTime = (*itWorker)->estimatedStartTime;
            if (nextTime > currentTime) {
                currentMinerals += (nextTime - currentTime) * (activeMineralWorker * mineralFactor);
                currentTime = nextTime;
            }
            ++activeMineralWorker;
            ++itWorker;
            finishTime = calcTime();
        }

        itResources->estimatedTime = currentTime + finishTime;
        currentMinerals -= neededMinerals;
    }
}
