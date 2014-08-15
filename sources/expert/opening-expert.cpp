#include "opening-expert.hpp"
#include "expert-registrar.hpp"

REGISTER_EXPERT(OpeningExpert)

bool OpeningExpert::tick(Blackboard* blackboard)
{
    blackboard->build(BWAPI::UnitTypes::Zerg_Spawning_Pool);
    return false;
}
