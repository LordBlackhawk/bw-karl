#pragma once

#include "basic-expert.hpp"

#include <vector>

class OverlordScoutingExpert : public BasicPortExpert
{
    public:
        void endTraversal() override;

        void visitProvideUnitPort(ProvideUnitPort* port) override;
        void visitResourceBoundaryItem(ResourceBoundaryItem* item) override;
        static bool isApplicable(Blackboard* blackboard);

    protected:
        std::vector<ProvideUnitPort*>   overlords;
        std::vector<ResourceBoundaryItem*>   minerals;
};
