#include "zerg-tech-expert.hpp"
#include "expert-registrar.hpp"
#include "plan/broodwar-boundary-items.hpp"
#include "plan/broodwar-plan-items.hpp"
#include "utils/log.hpp"

REGISTER_EXPERT(ZergTechExpert)

ZergTechExpert::ZergTechExpert()
    : techStarted(false)
{ }

void ZergTechExpert::beginTraversal()
{
    numberOfHatcharies = 0;
}

void ZergTechExpert::visitOwnHatcheryBoundaryItem(OwnHatcheryBoundaryItem* /*item*/)
{
    ++numberOfHatcharies;
}

/*
void ZergTechExpert::visitBuildPlanItem(BuildPlanItem* item)
{

}
*/

void ZergTechExpert::endTraversal()
{
    if ((numberOfHatcharies > 3) && !techStarted) {
        LOG << "Teching hydras...";
        currentBlackboard->build(BWAPI::UnitTypes::Zerg_Hydralisk_Den, {ResourceCategory::Tech});
        currentBlackboard->upgrade(BWAPI::UpgradeTypes::Muscular_Augments, {ResourceCategory::Tech}); // Speed
        currentBlackboard->upgrade(BWAPI::UpgradeTypes::Grooved_Spines, {ResourceCategory::Tech}); // Range
        currentBlackboard->upgrade(BWAPI::UpgradeTypes::Zerg_Missile_Attacks, {ResourceCategory::Tech});
        currentBlackboard->upgrade(BWAPI::UpgradeTypes::Zerg_Carapace, {ResourceCategory::Tech});
        techStarted = true;
    }
}
