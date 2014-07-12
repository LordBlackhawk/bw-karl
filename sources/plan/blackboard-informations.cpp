#include "blackboard-informations.hpp"

BaseLocation::BaseLocation()
    : origin(NULL)
{ }

BlackboardInformations::~BlackboardInformations()
{
    for (auto it : allBaseLocations)
        delete it;
}
