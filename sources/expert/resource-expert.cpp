#include "resource-expert.hpp"
#include "expert-registrar.hpp"
#include "plan/broodwar-ports.hpp"
#include "plan/broodwar-plan-items.hpp"
#include <algorithm>

REGISTER_EXPERT(ResourceExpert)

ResourceExpert::ResourceExpert()
    : counter(0), lastCollectedMinerals(0), lastCollectedGas(0)
{ }

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
    if (++counter % 13 == 1)
        resort();

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

void ResourceExpert::resort()
{
    auto info = currentBlackboard->getInformations();

    // Minerals:
    int newValue            = info->collectedMinerals;
    int newMinerals         = newValue - lastCollectedMinerals;
    lastCollectedMinerals   = newValue;

    // Gas:
    newValue            = info->collectedGas;
    int newGas          = newValue - lastCollectedGas;
    lastCollectedGas    = newValue;

    int sum = newMinerals + newGas;
    for (auto& it : info->resourceCategories)
        it.amount += it.ratio * sum;

    // Sort ports to category lists:
    EnumArray<std::vector<ResourcePort*>, ResourceCategory> categoryLists;
    for (auto it : resources)
        for (auto c : it->getCategory())
            categoryLists[c].push_back(it);

    // Copy amount of category:
    EnumArray<double, ResourceCategory> amount;
    EnumArray<unsigned int, ResourceCategory> indices;
    for (auto c : ResourceCategorySet::all()) {
        amount[c] = info->resourceCategories[c].amount;
        indices[c] = 0;
    }

    // Calculate new ordering (ratio != 0.0):
    std::set<ResourcePort*> done;
    resources.clear();
    while (true) {
        ResourceCategory bestCategory = ResourceCategory::Economy;
        ResourcePort*    bestPort = NULL;
        double           bestValue = 1e10;
        for (auto c : ResourceCategorySet::all()) {
            // Remove already taken elements
            auto& list = categoryLists[c];
            auto& index = indices[c];
            while (true) {
                if (index >= list.size())
                    break;
                if (done.find(list[index]) == done.end())
                    break;
                ++index;
            }
            if (index >= list.size())
                continue;

            auto& ratio = info->resourceCategories[c].ratio;
            assert(ratio != 0.0);
            double value = (list[index]->getMinerals() + list[index]->getGas() - amount[c]) / ratio;
            if (value < bestValue) {
                bestCategory = c;
                bestPort     = list[index];
                bestValue    = value;
            }
        }
        if (bestPort != NULL) {
            resources.push_back(bestPort);
            double sumRatio = 0.0;
            for (auto c : bestPort->getCategory())
                sumRatio += info->resourceCategories[c].ratio;
            for (auto c : bestPort->getCategory())
                amount[c] -= info->resourceCategories[c].ratio / sumRatio * (bestPort->getMinerals() + bestPort->getGas());
            ++indices[bestCategory];
            done.insert(bestPort);
        } else {
            break;
        }
    }
}
