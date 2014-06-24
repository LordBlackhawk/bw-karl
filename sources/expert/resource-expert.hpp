#pragma once

#include "basic-expert.hpp"

#include <vector>

class ResourceExpert : public BasicPortExpert
{
    public:
        void endTraversal() override;

        void visitResourcePort(ResourcePort* port) override;
        void visitGatherMineralPlanItem(GatherMineralsPlanItem* item) override;

    protected:
        std::vector<ResourcePort*>              resources;
        std::vector<GatherMineralsPlanItem*>    worker;

        void simulate();
};
