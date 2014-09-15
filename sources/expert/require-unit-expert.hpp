#pragma once

#include "basic-expert.hpp"

#include <vector>
#include <map>

class RequireUnitExpert : public BasicPortExpert
{
    public:
        void endTraversal() override;

        void visitProvideUnitPort(ProvideUnitPort* port) override;
        void visitRequireUnitPort(RequireUnitPort* port) override;

    protected:
        std::map<BWAPI::UnitType, std::vector<ProvideUnitPort*>>    provideMap;
        std::map<BWAPI::UnitType, std::vector<RequireUnitPort*>>    requireMap;
        bool activeUnitType(const BWAPI::UnitType unitType) const;
        void handleQueued(const BWAPI::UnitType unitType, const std::vector<ProvideUnitPort*>& provide, const std::vector<RequireUnitPort*>& require);
        void handleLarva(const std::vector<ProvideUnitPort*>& provide, const std::vector<RequireUnitPort*>& require);
        void handleCreepColony(std::vector<ProvideUnitPort*>& provide, std::vector<RequireUnitPort*>& require);
};
