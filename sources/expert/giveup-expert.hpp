#pragma once

#include "basic-expert.hpp"

class GiveUpExpert final : public BasicExpert
{
    public:
        GiveUpExpert();
        
        bool tick(Blackboard* blackboard) override;

        void endTraversal() override;

        void visitEnemyUnitBoundaryItem(EnemyUnitBoundaryItem* item);

    private:
        bool enemyNearby;
};
