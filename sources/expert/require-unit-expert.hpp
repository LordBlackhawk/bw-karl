#pragma once

#include "basic-expert.hpp"

#include <vector>
#include <map>
#include <queue>

struct OwnHatcherySorter
{
    bool operator () (OwnHatcheryBoundaryItem* lhs, OwnHatcheryBoundaryItem* rhs) const;
};

class RequireUnitExpert : public BasicPortExpert
{
    public:
        void beginTraversal() override;
        void endTraversal() override;

        void visitProvideUnitPort(ProvideUnitPort* port) override;
        void visitRequireUnitPort(RequireUnitPort* port) override;

        void visitOwnHatcheryBoundaryItem(OwnHatcheryBoundaryItem* item) override;

    protected:
        std::map<BWAPI::UnitType, std::vector<ProvideUnitPort*>>    provideMap;
        std::map<BWAPI::UnitType, std::vector<RequireUnitPort*>>    requireMap;
        std::priority_queue<OwnHatcheryBoundaryItem*, std::vector<OwnHatcheryBoundaryItem*>, OwnHatcherySorter> hatcheries;

        bool activeUnitType(const BWAPI::UnitType unitType) const;
        void handleQueued(const BWAPI::UnitType unitType, const std::vector<ProvideUnitPort*>& provide, const std::vector<RequireUnitPort*>& require);
        void handleLarva(std::vector<ProvideUnitPort*>& provide, const std::vector<RequireUnitPort*>& require);
        void handleCreepColony(std::vector<ProvideUnitPort*>& provide, std::vector<RequireUnitPort*>& require);
};
