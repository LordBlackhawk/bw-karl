#include "require-unit-expert.hpp"
#include "expert-registrar.hpp"
#include "plan/broodwar-ports.hpp"
#include "plan/broodwar-boundary-items.hpp"
#include "utils/log.hpp"
#include <algorithm>
#include <queue>

REGISTER_EXPERT(RequireUnitExpert)

bool OwnHatcherySorter::operator () (OwnHatcheryBoundaryItem* lhs, OwnHatcheryBoundaryItem* rhs) const
{
    return lhs->lastPlanedLarva() < rhs->lastPlanedLarva();
}

void RequireUnitExpert::beginTraversal()
{
    provideMap.clear();
    requireMap.clear();
}

bool RequireUnitExpert::activeUnitType(const BWAPI::UnitType unitType) const
{
    return unitType.isBuilding() || (unitType == BWAPI::UnitTypes::Zerg_Larva);
}

void RequireUnitExpert::visitProvideUnitPort(ProvideUnitPort* port)
{
    assert(!port->isActiveConnection());
    BWAPI::UnitType unitType = port->getUnitType();
    if (!activeUnitType(unitType))
        return;
    provideMap[unitType].push_back(port);
}

void RequireUnitExpert::visitRequireUnitPort(RequireUnitPort* port)
{
    assert(!port->isActiveConnection());
    BWAPI::UnitType unitType = port->getUnitType();
    if (!activeUnitType(unitType))
        return;
    requireMap[unitType].push_back(port);
}

void RequireUnitExpert::visitOwnHatcheryBoundaryItem(OwnHatcheryBoundaryItem* item)
{
    hatcheries.push(item);
    for (auto it : item->getLarvas())
        provideMap[BWAPI::UnitTypes::Zerg_Larva].push_back(&it->provideUnit);
}

void RequireUnitExpert::endTraversal()
{
    for (auto it : requireMap) {
        auto unitType = it.first;
        auto& require = it.second;
        auto& provide = provideMap[unitType];
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
    while (!hatcheries.empty())
        hatcheries.pop();
}

namespace
{
    struct RequireProvideUnitPair
    {
        RequireUnitPort*    require = NULL;
        ProvideUnitPort*    provide = NULL;
        bool operator < (const RequireProvideUnitPair& other) const
        {
            return require->estimatedTime < other.require->estimatedTime;
        }
    };
    struct ProviderWithTime
    {
        ProvideUnitPort*    provide;
        Time                estimatedTime;
        ProviderWithTime(ProvideUnitPort* p, Time t)
            : provide(p), estimatedTime(t)
        { }
        bool operator < (const ProviderWithTime& other) const
        {
            return estimatedTime > other.estimatedTime;
        }
    };
}

void RequireUnitExpert::handleQueued(const BWAPI::UnitType unitType, const std::vector<ProvideUnitPort*>& provide, const std::vector<RequireUnitPort*>& require)
{
    if ((provide.size() == 0) || (require.size() == 0))
        return;

    // 1. Collect items, which are owned by the same plan item.
    std::map<AbstractItem*, RequireProvideUnitPair>  items;
    for (auto it : provide)
        items[it->getOwner()].provide = it;
    for (auto it : require)
        items[it->getOwner()].require = it;

    // 2. There are simple provider and require/provider, but no only require. Sort them...
    std::priority_queue<ProviderWithTime> provider;
    std::vector<RequireProvideUnitPair> pairs;
    for (auto it : items) {
        auto& pair = it.second;
        if (((unitType == BWAPI::UnitTypes::Zerg_Hatchery) || (unitType == BWAPI::UnitTypes::Zerg_Lair)) && (pair.provide == NULL)) {
            auto planItem = dynamic_cast<MorphUnitPlanItem*>(pair.require->getOwner());
            if (planItem != NULL)
                pair.provide = &planItem->provideUnit;
        }
        assert(pair.provide != NULL);
        if (pair.require == NULL) {
            provider.push(ProviderWithTime(pair.provide, pair.provide->estimatedTime));
        } else {
            pairs.push_back(pair);
        }
    }
    std::sort(pairs.begin(), pairs.end());

    // 3. Create a provider if none exists:
    if (provider.size() == 0U)
        provider.push(ProviderWithTime(&currentBlackboard->build(unitType)->provideUnit, INFINITE_TIME));

    // 4. Distribute pairs among the provider:
    for (auto it : pairs) {
        auto& top = provider.top();
        Time estimatedTime = top.estimatedTime + it.provide->estimatedTime - it.require->estimatedTime;
        top.provide->connectTo(it.require);
        provider.pop();
        provider.push(ProviderWithTime(it.provide, estimatedTime));
    }
}

namespace
{
    class PortSorter
    {
        public:
            bool operator () (AbstractPort* lhs, AbstractPort* rhs) const
            {
                return lhs->estimatedTime < rhs->estimatedTime;
            }
    };
}

void RequireUnitExpert::handleLarva(std::vector<ProvideUnitPort*>& provide, const std::vector<RequireUnitPort*>& require)
{
    while ((provide.size() < require.size()) && !hatcheries.empty()) {
        auto hatch = hatcheries.top();
        hatcheries.pop();
        provide.push_back(hatch->createNewLarva());
        hatcheries.push(hatch);
    }
    std::sort(provide.begin(), provide.end(), PortSorter());
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
