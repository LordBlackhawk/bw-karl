#pragma once

#include "basic-expert.hpp"

#include <set>

class WorkerExpert : public BasicPortExpert
{
    public:
        bool tick(Blackboard* blackboard) override;

        void visitProvideUnitPort(ProvideUnitPort* port) override;
        void visitRequireUnitPort(RequireUnitPort* port) override;
        void visitGatherResourcesPlanItem(GatherResourcesPlanItem* item) override;

        void beginTraversal() override;
        void endTraversal() override;

    protected:
        Time                        timeHorizont;
        std::set<ProvideUnitPort*>  providePorts;

        ResourceBoundaryItem* findMineralForWorker(ProvideUnitPort* port);
};
