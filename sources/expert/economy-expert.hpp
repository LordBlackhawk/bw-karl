#pragma once

#include "basic-expert.hpp"

class EconomyExpert : public BasicExpert
{
    public:
        void beginTraversal() override;
        void endTraversal() override;

        void visitMorphUnitPlanItem(MorphUnitPlanItem* item) override;

    protected:
        int planedWorker;
        BWAPI::UnitType unitType;
};
