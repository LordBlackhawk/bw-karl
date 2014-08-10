#include "larva-expert.hpp"
#include "expert-registrar.hpp"
#include "plan/broodwar-ports.hpp"
#include <algorithm>

REGISTER_EXPERT(LarvaExpert)

bool LarvaExpert::isApplicable(Blackboard* /*blackboard*/)
{
    return BWAPI::Broodwar->self()->getRace() == BWAPI::Races::Zerg;
}

void LarvaExpert::visitProvideUnitPort(ProvideUnitPort* port)
{
    if (port->getUnitType() != BWAPI::UnitTypes::Zerg_Larva)
        return;
    provide.push_back(port);
}

void LarvaExpert::visitRequireUnitPort(RequireUnitPort* port)
{
    if (port->getUnitType() != BWAPI::UnitTypes::Zerg_Larva)
        return;
    require.push_back(port);
}

void LarvaExpert::endTraversal()
{
    //std::cout << "provide.size(): " << provide.size() << "; require.size(): " << require.size() << "\n";
    int size = std::min(provide.size(), require.size());
    for (int k=0; k<size; ++k)
        require[k]->connectTo(provide[k]);
    provide.clear();
    require.clear();
}
