#include "require-unit-expert.hpp"
#include "expert-registrar.hpp"
#include "plan/broodwar-ports.hpp"
#include <algorithm>

REGISTER_EXPERT(RequireUnitExpert)

bool RequireUnitExpert::activeUnitType(const BWAPI::UnitType unitType) const
{
    return unitType.isBuilding() || (unitType == BWAPI::UnitTypes::Zerg_Larva);
}

void RequireUnitExpert::visitProvideUnitPort(ProvideUnitPort* port)
{
    BWAPI::UnitType unitType = port->getUnitType();
    if (!activeUnitType(unitType) || port->isImpossible())
        return;
    provideMap[unitType].push_back(port);
}

void RequireUnitExpert::visitRequireUnitPort(RequireUnitPort* port)
{
    BWAPI::UnitType unitType = port->getUnitType();
    if (!activeUnitType(unitType))
        return;
    requireMap[unitType].push_back(port);
}

void RequireUnitExpert::endTraversal()
{
    for (auto it : requireMap) {
        auto unitType = it.first;
        auto& require = it.second;
        auto& provide = provideMap[unitType];
        //std::cout << "provide.size(): " << provide.size() << "; require.size(): " << require.size() << "\n";
        if (unitType == BWAPI::UnitTypes::Zerg_Larva) {
            handleLarva(provide, require);
        } else if (unitType == BWAPI::UnitTypes::Zerg_Creep_Colony) {
            handleCreepColony(provide, require);
        } else {
            handleQueued(unitType, provide, require);
        }
    }
    currentBlackboard->getInformations()->unusedLarvaCount = std::max(0, ((int)provideMap[BWAPI::UnitTypes::Zerg_Larva].size()-(int)requireMap[BWAPI::UnitTypes::Zerg_Larva].size()));
    provideMap.clear();
    requireMap.clear();
}

void RequireUnitExpert::handleQueued(const BWAPI::UnitType /*unitType*/, const std::vector<ProvideUnitPort*>& /*provide*/, const std::vector<RequireUnitPort*>& /*require*/)
{
    assert( false && "not yet implemented!" );
}

void RequireUnitExpert::handleLarva(const std::vector<ProvideUnitPort*>& provide, const std::vector<RequireUnitPort*>& require)
{
    int size = std::min(provide.size(), require.size());
    for (int k=0; k<size; ++k)
        require[k]->connectTo(provide[k]);
}

namespace
{
    template <class T>
    void removeConnected(std::vector<T*>& v)
    {
        v.erase(std::remove_if(v.begin(), v.end(), [] (T* item) {
                return item->isConnected();
            }), v.end());
    }
}

void RequireUnitExpert::handleCreepColony(std::vector<ProvideUnitPort*>& provide, std::vector<RequireUnitPort*>& require)
{
    removeConnected(provide);
    removeConnected(require);
    unsigned int provideSize = provide.size();
    unsigned int requireSize = require.size();
    for (unsigned int k=0; k<requireSize; ++k) {
        ProvideUnitPort* port;
        if (k < provideSize) {
            port = provide[k];
        } else {
            auto planItem = currentBlackboard->build(BWAPI::UnitTypes::Zerg_Creep_Colony);
            port = &planItem->provideUnit;
            planItem->addPurpose(port, dynamic_cast<AbstractPlanItem*>(require[k]->getOwner()));
        }
        require[k]->connectTo(port);
    }
}
