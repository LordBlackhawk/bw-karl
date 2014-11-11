#include "expand-expert.hpp"
#include "expert-registrar.hpp"
#include "plan/broodwar-boundary-items.hpp"
#include "plan/broodwar-plan-items.hpp"
#include "utils/log.hpp"

REGISTER_EXPERT(ExpandExpert)

void ExpandExpert::beginTraversal()
{
    hasExpandInPlan = false;
    auto info = currentBlackboard->getInformations();
    unitType = info->selfRace.getCenter();
}

void ExpandExpert::visitBuildPlanItem(BuildPlanItem* item)
{
    if ((item->getUnitType() == unitType) && !item->isActive())
        hasExpandInPlan = true;
}

void ExpandExpert::endTraversal()
{
    if (!hasExpandInPlan) {
        auto item = currentBlackboard->build(unitType, {ResourceCategory::Expansion});
        if ((double)rand() / (double)RAND_MAX < 0.7)
            item->requireSpace.connectTo(getExpandPosition());
    }

    auto info = currentBlackboard->getInformations();
    unsigned int numberOfBases = info->ownBaseLocations.size();
    info->ownBaseLocations.clear();
    for (auto it : info->allBaseLocations)
        if (dynamic_cast<OwnUnitBoundaryItem*>(it->getBaseUnit()) != NULL)
            info->ownBaseLocations.insert(it);
    if (numberOfBases != info->ownBaseLocations.size())
        LOG << "Number of bases changed from " << numberOfBases << " to " << info->ownBaseLocations.size() << ".";
}

BWAPI::TilePosition ExpandExpert::getExpandPosition() const
{
    auto info = currentBlackboard->getInformations();
    BaseLocation* bestLocation = NULL;
    double bestDistance = 1e10;
    for (auto it : info->allBaseLocations) {
        if (it->isOccupied())
            continue;
        double distance = 1e10;
        for (auto obl : info->ownBaseLocations)
            distance = std::min(distance, BWTA::getGroundDistance(it->getTilePosition(), obl->getTilePosition()));
        if (distance < bestDistance) {
            bestDistance = distance;
            bestLocation = it;
        }
    }
    if (bestLocation == NULL) {
        LOG << "No expand location found!!!";
        return BWAPI::TilePositions::None;
    }
    return bestLocation->getTilePosition();
}
