#pragma once

#include "basic-expert.hpp"

class ZergOpeningExpert final : public BasicExpert
{
    public:
        bool tick(Blackboard* blackboard) override;

        static bool isApplicable(Blackboard* blackboard);
};
