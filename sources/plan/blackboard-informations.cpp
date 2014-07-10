#include "blackboard-informations.hpp"

BlackboardInformations::~BlackboardInformations()
{
    for (auto it : allBaseLocations)
        delete it;
}
