#include "spawn-lings-expert.hpp"
#include "expert-registrar.hpp"
#include "plan/broodwar-boundary-items.hpp"
#include "plan/broodwar-plan-items.hpp"

REGISTER_EXPERT(SpawnLingsExpert)

bool SpawnLingsExpert::isApplicable(Blackboard* /*blackboard*/)
{
    return BWAPI::Broodwar->self()->getRace() == BWAPI::Races::Zerg;
}

void SpawnLingsExpert::beginTraversal()
{
    hasPool = false;
    hasHydraliskDen = false;
    numberOfLarva = 0;
    numberOfMorphLings = 0;
    numberOfHydralisks = 0;
}

void SpawnLingsExpert::visitOwnUnitBoundaryItem(OwnUnitBoundaryItem* item)
{
    if (item->getUnitType() == BWAPI::UnitTypes::Zerg_Spawning_Pool)
        hasPool = true;
    if (item->getUnitType() == BWAPI::UnitTypes::Zerg_Larva)
        ++numberOfLarva;
    if (item->getUnitType() == BWAPI::UnitTypes::Zerg_Hydralisk_Den)
        hasHydraliskDen = true;
}

void SpawnLingsExpert::visitMorphUnitPlanItem(MorphUnitPlanItem* item)
{
    if ((item->getUnitType() == BWAPI::UnitTypes::Zerg_Zergling) && !item->isActive())
        ++numberOfMorphLings;
    if ((item->getUnitType() == BWAPI::UnitTypes::Zerg_Hydralisk) && !item->isActive())
        ++numberOfHydralisks;
}

void SpawnLingsExpert::endTraversal()
{
    if (hasPool && (numberOfMorphLings < numberOfLarva+2))
        currentBlackboard->morph(BWAPI::UnitTypes::Zerg_Zergling, {ResourceCategory::Units});
    if (hasHydraliskDen && (numberOfHydralisks < numberOfLarva+2))
        currentBlackboard->morph(BWAPI::UnitTypes::Zerg_Hydralisk, {ResourceCategory::Units});
}
