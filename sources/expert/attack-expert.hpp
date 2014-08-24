#pragma once

#include "basic-expert.hpp"

#include <vector>

class AttackExpert : public BasicPortExpert
{
    public:
        void endTraversal() override;

        void visitOwnUnitBoundaryItem(OwnUnitBoundaryItem* item) override;
        void visitEnemyUnitBoundaryItem(EnemyUnitBoundaryItem* item) override;

        static bool isApplicable(Blackboard* blackboard);

    protected:
        std::vector<ProvideUnitPort*>       lings;
        std::vector<EnemyUnitBoundaryItem*> enemies;

        double valueEnemyUnit(ProvideUnitPort* ling, EnemyUnitBoundaryItem* enemy);
};
