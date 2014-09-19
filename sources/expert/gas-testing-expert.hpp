#pragma once

#include "basic-expert.hpp"

class GasTestingExpert final : public BasicPortExpert
{
    public:

        void beginTraversal() override;
        void endTraversal() override;
        //bool tick(Blackboard* blackboard) override;
        void visitResourcePort(ResourcePort *port) override;
        void visitProvideResourcePort(ProvideResourcePort *port) override;
        void visitResourceBoundaryItem(ResourceBoundaryItem* item) override;
        void visitProvideUnitExistancePort(ProvideUnitExistancePort* port) override;
        void visitBuildPlanItem(BuildPlanItem* item) override;

        static bool isApplicable(Blackboard* blackboard);

    protected:
        std::vector<ResourceBoundaryItem*>   overlords;
        bool needGas;
        bool haveGas;
};
