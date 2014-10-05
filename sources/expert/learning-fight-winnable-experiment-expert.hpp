#pragma once

#include "basic-expert.hpp"

class LearningFightWinnableExperimentExpert final : public BasicPortExpert
{
    public:
        void beginTraversal() override;
        void endTraversal() override;
        void visitProvideUnitPort(ProvideUnitPort* port) override;
        void visitOwnUnitBoundaryItem(OwnUnitBoundaryItem* item) override;
        void visitEnemyUnitBoundaryItem(EnemyUnitBoundaryItem* item) override;

        static bool isApplicable(Blackboard* blackboard);
        
        void matchEnd(Blackboard* blackboard) override;
    private:
        std::set<OwnUnitBoundaryItem*> ownUnits;
        std::set<EnemyUnitBoundaryItem*> enemyUnits;
};
