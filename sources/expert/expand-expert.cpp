#include "expand-expert.hpp"
#include "expert-registrar.hpp"
#include "plan/broodwar-boundary-items.hpp"
#include "plan/broodwar-plan-items.hpp"

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
    if (!hasExpandInPlan)
        currentBlackboard->build(unitType, {ResourceCategory::Expansion});
}
