#include "economy-expert.hpp"
#include "expert-registrar.hpp"
#include "plan/broodwar-boundary-items.hpp"
#include "plan/broodwar-plan-items.hpp"
#include "utils/log.hpp"

REGISTER_EXPERT(EconomyExpert)

void EconomyExpert::beginTraversal()
{
    planedWorker = 0;
    auto info = currentBlackboard->getInformations();
    unitType = info->selfRace.getWorker();
}

void EconomyExpert::visitMorphUnitPlanItem(MorphUnitPlanItem* item)
{
    if ((item->getUnitType() == unitType) && !item->isActive())
        ++planedWorker;
}

void EconomyExpert::endTraversal()
{
    if ((planedWorker < 2) && (currentBlackboard->getLastUpdateTime() > 10))
        currentBlackboard->create(unitType, {ResourceCategory::Economy});
}
