#pragma once

#include "basic-expert.hpp"

class OpeningExpert final : public BasicExpert
{
    public:
        OpeningExpert();

        void prepare() override;
        bool tick(Blackboard* blackboard) override;

    protected:
        BWAPI::TilePosition poolPos;
};
