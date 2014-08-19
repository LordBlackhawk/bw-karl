#include "opening-expert.hpp"
#include "expert-registrar.hpp"

REGISTER_EXPERT(OpeningExpert)

bool OpeningExpert::tick(Blackboard* /*blackboard*/)
{
    return false;
}
