#include "opening-expert.hpp"
#include "expert-registrar.hpp"

REGISTER_EXPERT(OpeningExpert)

bool OpeningExpert::tick(Blackboard* blackboard)
{
    blackboard->createBuildPlanItem(BWAPI::UnitTypes::Zerg_Spawning_Pool);
    return false;
}
