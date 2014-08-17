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
    numberOfLarva = 0;
    numberOfMorphLings = 0;
}

void SpawnLingsExpert::visitOwnUnitBoundaryItem(OwnUnitBoundaryItem* item)
{
    if (item->getUnitType() == BWAPI::UnitTypes::Zerg_Spawning_Pool)
        hasPool = true;
    if (item->getUnitType() == BWAPI::UnitTypes::Zerg_Larva)
        ++numberOfLarva;
}

void SpawnLingsExpert::visitMorphUnitPlanItem(MorphUnitPlanItem* item)
{
    if ((item->getUnitType() == BWAPI::UnitTypes::Zerg_Zergling) && !item->isActive())
        ++numberOfMorphLings;
}

void SpawnLingsExpert::endTraversal()
{
    std::cout << "hasPool: " << (hasPool ? "true" : "false")
        << "; numberOfMorphLings: " << numberOfMorphLings
        << "; numberOfLarva: " << numberOfLarva << "\n";
    if (hasPool && (numberOfMorphLings < numberOfLarva+2))
        currentBlackboard->morph(BWAPI::UnitTypes::Zerg_Zergling);
}
