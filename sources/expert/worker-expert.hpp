#pragma once

#include "basic-expert.hpp"

#include <set>

class WorkerExpert : public BasicPortExpert
{
    public:
        void visitProvideUnitPort(ProvideUnitPort* port) override;
        void visitRequireUnitPort(RequireUnitPort* port) override;

        void endTraversal() override;

    protected:
        std::set<ProvideUnitPort*>  providePorts;
};
