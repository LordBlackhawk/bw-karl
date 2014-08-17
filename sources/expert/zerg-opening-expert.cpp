#include "zerg-opening-expert.hpp"
#include "expert-registrar.hpp"

REGISTER_EXPERT(ZergOpeningExpert)

bool ZergOpeningExpert::isApplicable(Blackboard* /*blackboard*/)
{
    return BWAPI::Broodwar->self()->getRace() == BWAPI::Races::Zerg;
}

bool ZergOpeningExpert::tick(Blackboard* blackboard)
{
    blackboard->morph(BWAPI::UnitTypes::Zerg_Drone);
    blackboard->build(BWAPI::UnitTypes::Zerg_Spawning_Pool);
    return false;
}
