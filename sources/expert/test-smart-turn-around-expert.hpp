#pragma once

#include "basic-expert.hpp"

class TestSmartTurnAroundExpert : public BasicExpert
{
    public:
        void visitOwnUnitBoundaryItem(OwnUnitBoundaryItem* item) override;
};
