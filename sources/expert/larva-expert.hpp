#pragma once

#include "basic-expert.hpp"

#include <vector>

class LarvaExpert : public BasicPortExpert
{
    public:
        void endTraversal() override;

        void visitProvideUnitPort(ProvideUnitPort* port) override;
        void visitRequireUnitPort(RequireUnitPort* port) override;

        static bool isApplicable(Blackboard* blackboard);

    protected:
        std::vector<ProvideUnitPort*>   provide;
        std::vector<RequireUnitPort*>   require;
};
