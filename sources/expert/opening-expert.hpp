#pragma once

#include "basic-expert.hpp"

class OpeningExpert final : public BasicExpert
{
    public:
        OpeningExpert();

        void prepare() override;
        void tick(Blackboard* blackboard) override;

    protected:
        BWAPI::TilePosition poolPos;
};
