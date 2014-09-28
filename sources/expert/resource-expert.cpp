#include "resource-expert.hpp"
#include "expert-registrar.hpp"
#include "plan/broodwar-ports.hpp"
#include "plan/broodwar-plan-items.hpp"
#include <algorithm>

REGISTER_EXPERT(ResourceExpert)

void ResourceExpert::visitResourcePort(ResourcePort* port)
{
    resources.push_back(port);
}

void ResourceExpert::visitGatherResourcesPlanItem(GatherResourcesPlanItem* item)
{
    if (item->isImpossible())
        return;
    if (item->isGatherMinerals()) {
        workerMinerals.push_back(item);
    } else {
        workerGas.push_back(item);
    }
}

void ResourceExpert::endTraversal()
{
    simulateMinerals();
    simulateGas();
    resources.clear();
    workerMinerals.clear();
    workerGas.clear();
}

void ResourceExpert::simulateMinerals()
{
    const double mineralFactor = 0.045;
    Time currentTime = currentBlackboard->getLastUpdateTime();
    double currentMinerals = currentBlackboard->getInformations()->currentMinerals;
    int activeMineralWorker = 0;

    auto itWorker = workerMinerals.begin();
    for (auto itResources : resources) {
        int neededMinerals = itResources->getMinerals();
        Time finishTime = INFINITE_TIME;
        if (neededMinerals <= currentMinerals) {
            finishTime = currentTime;
        } else {
            while (true) {
                if (activeMineralWorker > 0)
                    finishTime = currentTime + Time((neededMinerals - currentMinerals) / (activeMineralWorker * mineralFactor));

                if (itWorker == workerMinerals.end())
                    break;
                Time nextTime = (*itWorker)->estimatedStartTime;
                if (finishTime <= nextTime)
                    break;

                if (nextTime > currentTime) {
                    currentMinerals += (nextTime - currentTime) * (activeMineralWorker * mineralFactor);
                    currentTime = nextTime;
                }
                ++activeMineralWorker;
                ++itWorker;
            }

            finishTime = std::max(finishTime, currentBlackboard->getActionHorizon());
        }

        itResources->estimatedTime = finishTime;
        currentMinerals -= neededMinerals;
    }
}

void ResourceExpert::simulateGas()
{
    const double gasFactor = 0.072;
    Time currentTime = currentBlackboard->getLastUpdateTime();
    double currentGas = currentBlackboard->getInformations()->currentGas;
    int activeGasWorker = 0;

    auto itWorker = workerGas.begin();
    for (auto itResources : resources) {
        int neededGas = itResources->getGas();
        if (neededGas == 0)
            continue;

        Time finishTime = INFINITE_TIME;
        if (neededGas <= currentGas) {
            finishTime = currentTime;
        } else {
            while (true) {
                if (activeGasWorker > 0)
                    finishTime = currentTime + Time((neededGas - currentGas) / (activeGasWorker * gasFactor));

                if (itWorker == workerGas.end())
                    break;
                Time nextTime = (*itWorker)->estimatedStartTime;
                if (finishTime <= nextTime)
                    break;

                if (nextTime > currentTime) {
                    currentGas += (nextTime - currentTime) * (activeGasWorker * gasFactor);
                    currentTime = nextTime;
                }
                ++activeGasWorker;
                ++itWorker;
            }

            finishTime = std::max(finishTime, currentBlackboard->getActionHorizon());
        }

        itResources->estimatedTime = std::max(itResources->estimatedTime, finishTime);
        currentGas -= neededGas;
    }
}
