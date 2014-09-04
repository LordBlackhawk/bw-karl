#pragma once

#include "basic-expert.hpp"
#include <vector>

class AbstractSpaceUnitBoundaryItem;

class GuerillaExpert : public BasicExpert
{
    public:
        void beginTraversal() override;
        void endTraversal() override;

        void visitOwnUnitBoundaryItem(OwnUnitBoundaryItem* item) override;
        void visitEnemyUnitBoundaryItem(EnemyUnitBoundaryItem* item) override;

    protected:
        std::vector<AbstractSpaceUnitBoundaryItem*>   allUnits;
        void analyzeSituation(const std::vector<AbstractSpaceUnitBoundaryItem*>& units);
        void retreat(const std::vector<OwnUnitBoundaryItem*>& ownUnits, const std::vector<EnemyUnitBoundaryItem*>& enemyUnits);
        void retreatTo(OwnUnitBoundaryItem* ownUnit, BWAPI::Position pos);
};
