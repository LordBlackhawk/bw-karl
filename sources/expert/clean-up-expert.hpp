#pragma once

#include "basic-expert.hpp"

class CleanUpExpert : public BasicPortExpert
{
    public:
        void visitMoveToPositionPlanItem(MoveToPositionPlanItem* item) override;
        void visitAttackUnitPlanItem(AttackUnitPlanItem* item) override;
};
