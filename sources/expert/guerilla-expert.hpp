#pragma once

#include "basic-expert.hpp"
#include <vector>

class AbstractSpaceUnitBoundaryItem;

class GuerillaExpert : public BasicExpert
{
    public:
        void endTraversal() override;

        void visitOwnUnitBoundaryItem(OwnUnitBoundaryItem* item) override;
        void visitEnemyUnitBoundaryItem(EnemyUnitBoundaryItem* item) override;

    protected:
        std::vector<AbstractSpaceUnitBoundaryItem*>   allUnits;
        void analyzeSituation(int clusterIndex, const std::vector<AbstractSpaceUnitBoundaryItem*>& units);
        void cleanup(const std::vector<OwnUnitBoundaryItem*>& ownUnits);
        void retreat(const std::vector<OwnUnitBoundaryItem*>& ownUnits, const std::vector<EnemyUnitBoundaryItem*>& enemyUnits);
        void retreatTo(OwnUnitBoundaryItem* ownUnit, BWAPI::Position pos);
};

struct Obstacle
{
    double direction;
    double width;
    Obstacle(double d, double w)
        : direction(d), width(w)
    { }
    bool operator < (const Obstacle& other) const
    {
        return direction < other.direction;
    }
};

class ObstacleSolver
{
    public:
        void add(double direction, double width);
        void add(const BWAPI::Position& relPosition, BWAPI::UnitType unitType);
        void add(const BWAPI::Position& tileCenter);
        BWAPI::Position solve();

    private:
        std::vector<Obstacle>   obstacles;
};
