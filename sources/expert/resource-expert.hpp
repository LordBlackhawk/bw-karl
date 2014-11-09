#pragma once

#include "basic-expert.hpp"

#include <vector>

class ResourceExpert : public BasicPortExpert
{
    public:
        ResourceExpert();

        void endTraversal() override;

        void visitResourcePort(ResourcePort* port) override;
        void visitGatherResourcesPlanItem(GatherResourcesPlanItem* item) override;

    protected:
        int counter;
        int lastCollectedMinerals;
        int lastCollectedGas;
        std::vector<ResourcePort*>              resources;
        std::vector<GatherResourcesPlanItem*>   workerMinerals;
        std::vector<GatherResourcesPlanItem*>   workerGas;

        void simulateMinerals();
        void simulateGas();
        void resort();
};
