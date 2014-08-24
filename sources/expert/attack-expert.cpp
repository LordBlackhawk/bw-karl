#include "attack-expert.hpp"
#include "expert-registrar.hpp"
#include "plan/broodwar-boundary-items.hpp"
#include "plan/broodwar-plan-items.hpp"
#include "utils/random-chooser.hpp"

REGISTER_EXPERT(AttackExpert)

bool AttackExpert::isApplicable(Blackboard* /*blackboard*/)
{
    return BWAPI::Broodwar->self()->getRace() == BWAPI::Races::Zerg;
}

void AttackExpert::visitOwnUnitBoundaryItem(OwnUnitBoundaryItem* item)
{
    if ((item->getUnitType() != BWAPI::UnitTypes::Zerg_Zergling) || item->isConnected())
        return;
    lings.push_back(&item->provideUnit);
}

void AttackExpert::visitEnemyUnitBoundaryItem(EnemyUnitBoundaryItem* item)
{
    if (item->getUnitType().isFlyer())
        return;
    enemies.push_back(item);
}

void AttackExpert::endTraversal()
{
    for (auto ling : lings) {
        BWAPI::Position bestPosition = BWAPI::Positions::Unknown;
        double          bestValue = -1.0e10;

        for (auto enemy : enemies) {
            double value = valueEnemyUnit(ling, enemy);
            if (value > bestValue) {
                bestPosition = enemy->getPosition();
                bestValue = value;
            }
        }

        // If no enemy known, then search at random unexplored base location.
        if (bestPosition == BWAPI::Positions::Unknown) {
            std::set<BaseLocation*> unexplored;
            for (auto it : currentBlackboard->getInformations()->allBaseLocations)
                if (!it->isCompleteExplored())
                    unexplored.insert(it);
            auto location = getRandomItem(unexplored);
            if (location != NULL)
                bestPosition = location->getPosition();
        }

        if (bestPosition != BWAPI::Positions::Unknown)
            currentBlackboard->attack(ling, bestPosition);
    }
    lings.clear();
    enemies.clear();
}

double AttackExpert::valueEnemyUnit(ProvideUnitPort* ling, EnemyUnitBoundaryItem* enemy)
{
    double distance = ling->getPosition().getDistance(enemy->getPosition());
    double value = 0.0;
    if (enemy->getUnitType().isBuilding())
        value += 500.0;
    if (enemy->getUnitType().isWorker())
        value += 100.0;
    return value - distance;
}
