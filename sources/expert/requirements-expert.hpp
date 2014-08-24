#pragma once

#include "basic-expert.hpp"

class RequirementsExpert : public BasicPortExpert
{
    public:
        void endTraversal() override;

        void visitRequireUnitExistancePort(RequireUnitExistancePort* port) override;
        void visitOwnUnitBoundaryItem(OwnUnitBoundaryItem* item) override;
        void visitMorphUnitPlanItem(MorphUnitPlanItem* item) override;
        void visitBuildPlanItem(BuildPlanItem* item) override;

    protected:
        std::vector<RequireUnitExistancePort*>      requirePorts;
        std::map<BWAPI::UnitType, AbstractItem*>    firstProvider;
        void addProvider(BWAPI::UnitType ut, AbstractItem* item);
};
