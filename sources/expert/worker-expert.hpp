#pragma once

#include "basic-expert.hpp"

#include <set>

class WorkerExpert : public BasicPortExpert
{
    public:
        bool tick(Blackboard* blackboard) override;

        void visitProvideUnitPort(ProvideUnitPort* port) override;
        void visitRequireUnitPort(RequireUnitPort* port) override;

        void endTraversal() override;

    protected:
        std::set<ProvideUnitPort*>  providePorts;
};
