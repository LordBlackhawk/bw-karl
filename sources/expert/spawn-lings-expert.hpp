#pragma once

#include "basic-expert.hpp"

class SpawnLingsExpert : public BasicExpert
{
    public:
        void beginTraversal() override;
        void endTraversal() override;

        void visitOwnUnitBoundaryItem(OwnUnitBoundaryItem* item) override;
        void visitMorphUnitPlanItem(MorphUnitPlanItem* item) override;

        static bool isApplicable(Blackboard* blackboard);

    protected:
        bool hasPool;
        bool hasHydraliskDen;
        int numberOfLarva;
        int numberOfMorphLings;
        int numberOfHydralisks;
};
