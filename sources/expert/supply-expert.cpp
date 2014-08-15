#include "supply-expert.hpp"
#include "expert-registrar.hpp"
#include "plan/broodwar-ports.hpp"
#include "utils/log.hpp"

REGISTER_EXPERT(SupplyExpert)

void SupplyExpert::visitSupplyPort(SupplyPort* port)
{
    if (port->getRace() == BWAPI::Races::Terran) {
        if (port->isRequirePort()) {
            requireTerran.push_back(port);
        } else {
            provideTerran.push_back(port);
        }
    } else if (port->getRace() == BWAPI::Races::Zerg) {
        if (port->isRequirePort()) {
            requireZerg.push_back(port);
        } else {
            provideZerg.push_back(port);
        }
    } else if (port->getRace() == BWAPI::Races::Protoss) {
        if (port->isRequirePort()) {
            requireProtoss.push_back(port);
        } else {
            provideProtoss.push_back(port);
        }
    }
}

namespace
{
    bool process(std::vector<SupplyPort*>& provide, std::vector<SupplyPort*>& require)
    {
        //LOG << "provide.size(): " << provide.size() << "; require.size(): " << require.size();
        Time currentTime = 0;
        int currentSupply = 0;
        auto itProvide = provide.begin();
        bool result = false;
        for (auto itRequire : require) {
            while ((itProvide != provide.end()) && (currentSupply < itRequire->getRequiredAmount())) {
                currentTime = (*itProvide)->estimatedTime;
                currentSupply += (*itProvide)->getProvidedAmount();
                ++itProvide;
            }
            if (result || (currentSupply < itRequire->getRequiredAmount())) {
                itRequire->estimatedTime = INFINITE_TIME;
                result = true;
            } else {
                itRequire->estimatedTime = currentTime;
                currentSupply -= itRequire->getRequiredAmount();
            }
        }
        //LOG << "supply remaining: " << currentSupply;
        provide.clear();
        require.clear();
        return result;
    }
}

void SupplyExpert::endTraversal()
{
    if (process(provideTerran, requireTerran))
        currentBlackboard->build(BWAPI::UnitTypes::Terran_Supply_Depot);
    if (process(provideZerg, requireZerg))
        currentBlackboard->morph(BWAPI::UnitTypes::Zerg_Overlord);
    if (process(provideProtoss, requireProtoss))
        currentBlackboard->build(BWAPI::UnitTypes::Protoss_Pylon);
}
