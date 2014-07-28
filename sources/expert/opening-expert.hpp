#pragma once

#include "basic-expert.hpp"

class OpeningExpert final : public BasicExpert
{
    public:
        bool tick(Blackboard* blackboard) override;
};
