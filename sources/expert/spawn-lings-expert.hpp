#pragma once

#include "basic-expert.hpp"

#include <vector>

class SpawnLingsExpert : public BasicPortExpert
{
    public:
        void beginTraversal() override;
        void endTraversal() override;

        void visitOwnUnitBoundaryItem(OwnUnitBoundaryItem* item) override;
        void visitMorphUnitPlanItem(MorphUnitPlanItem* item) override;

        static bool isApplicable(Blackboard* blackboard);

    protected:
        bool hasPool;
        int numberOfLarva;
        int numberOfMorphLings;
};
