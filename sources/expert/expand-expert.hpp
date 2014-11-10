#pragma once

#include "basic-expert.hpp"

class ExpandExpert : public BasicExpert
{
    public:
        void beginTraversal() override;
        void endTraversal() override;

        void visitBuildPlanItem(BuildPlanItem* item) override;

    protected:
        bool hasExpandInPlan;
        BWAPI::UnitType unitType;

        BWAPI::TilePosition getExpandPosition() const;
};
