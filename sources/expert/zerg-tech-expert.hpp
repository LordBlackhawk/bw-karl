#pragma once

#include "basic-expert.hpp"

class ZergTechExpert : public BasicExpert
{
    public:
        ZergTechExpert();

        void beginTraversal() override;
        void endTraversal() override;

        void visitOwnHatcheryBoundaryItem(OwnHatcheryBoundaryItem* item) override;
        //void visitBuildPlanItem(BuildPlanItem* item) override;

    protected:
        int numberOfHatcharies;
        bool techStarted;
};
