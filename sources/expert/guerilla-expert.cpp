#include "guerilla-expert.hpp"
#include "expert-registrar.hpp"
#include "plan/broodwar-boundary-items.hpp"
#include "utils/bw-helper.hpp"
#include "utils/options.hpp"
#include <algorithm>

#define M_PI 3.141592653589793238462643383279502884L

REGISTER_EXPERT(GuerillaExpert)

void GuerillaExpert::visitOwnUnitBoundaryItem(OwnUnitBoundaryItem* item)
{
    if (    item->isBuilding()
         || item->isFlying()
         || !item->getPosition().isValid()
         || (item->getUnitType() == BWAPI::UnitTypes::Zerg_Larva)
         || (item->getUnitType() == BWAPI::UnitTypes::Zerg_Egg)
        )
        return;
    allUnits.push_back(item);
}

void GuerillaExpert::visitEnemyUnitBoundaryItem(EnemyUnitBoundaryItem* item)
{
    if (    item->isBuilding()
         || item->isFlying()
         || !item->getPosition().isValid()
         || (item->getUnitType() == BWAPI::UnitTypes::Zerg_Larva)
         || (item->getUnitType() == BWAPI::UnitTypes::Zerg_Egg)
         || !item->isVisible()
         || item->isGatheringMinerals()
         || item->isGatheringGas()
        )
        return;
    allUnits.push_back(item);
}

// 1. Cluster units
void GuerillaExpert::endTraversal()
{
    int index = 0;
    while (!allUnits.empty()) {
        std::vector<AbstractSpaceUnitBoundaryItem*> cluster = { allUnits.back() };
        allUnits.pop_back();
        for (unsigned int k=0; k<cluster.size(); ++k) {
            auto position = cluster[k]->getPosition();
            allUnits.erase(std::remove_if(allUnits.begin(), allUnits.end(), [&](AbstractSpaceUnitBoundaryItem* unit) {
                    return (position.getDistance(unit->getPosition()) < 128.0) && (cluster.push_back(unit), true);
                }), allUnits.end());
        }
        analyzeSituation(index++, cluster);
    }

    allUnits.clear();
}

// 2. Classify situations
namespace
{
    template <class T>
    double valueOfUnits(std::vector<T*>& units)
    {
        std::sort(units.begin(), units.end(), [](T* lhs, T* rhs) {
                return (lhs->getHealth() > rhs->getHealth());
            });
        double value = 0.0;
        double strength = 0.0;
        for (auto it : units) {
            strength += it->getGroundDPS();
            value += it->getHealth() * strength;
        }
        return value;
    }
}

void GuerillaExpert::analyzeSituation(int clusterIndex, const std::vector<AbstractSpaceUnitBoundaryItem*>& units)
{
    std::vector<OwnUnitBoundaryItem*> ownUnits;
    std::vector<EnemyUnitBoundaryItem*> enemyUnits;
    for (auto it : units) {
        auto ou = dynamic_cast<OwnUnitBoundaryItem*>(it);
        if (ou != NULL) {
            ownUnits.push_back(ou);
        } else {
            auto eu = dynamic_cast<EnemyUnitBoundaryItem*>(it);
            if (eu != NULL)
                enemyUnits.push_back(eu);
        }
    }

    if (ownUnits.empty() || enemyUnits.empty()) {
        cleanup(ownUnits);
        return;
    }

    if (OptionsRegistrar::optHUD() && !OptionsRegistrar::optParallel()) {
        static BWAPI::Color colors[] = { BWAPI::Colors::Red, BWAPI::Colors::Green, BWAPI::Colors::Yellow, BWAPI::Colors::Teal, BWAPI::Colors::Purple, 
                                        BWAPI::Colors::Orange, BWAPI::Colors::Brown, BWAPI::Colors::White, BWAPI::Colors::Blue, BWAPI::Colors::Cyan, BWAPI::Colors::Grey };
        static int numberOfColors = sizeof(colors) / sizeof(BWAPI::Color);
        for (auto it : units) {
            BWAPI::Position pos = it->getPosition();
            BWAPI::Broodwar->drawCircleMap(pos.x(), pos.y(), 20, colors[clusterIndex % numberOfColors]);
        }
    }

    double ownPower = valueOfUnits(ownUnits);
    double enemyPower = valueOfUnits(enemyUnits);

    if (ownPower > 0.9 * enemyPower) {
        cleanup(ownUnits);
        return;
    }

    //std::cout << currentBlackboard->getLastUpdateTime() << ": Retreating " << ownUnits.size() << " units from " << enemyUnits.size() << " enemy units...\n";
    retreat(ownUnits, enemyUnits);
}

// 3. and action ...
namespace
{
    double angleOf(const BWAPI::Position& pos)
    {
        return atan2((double)pos.y(), (double)pos.x());
    }

    double angleDistance(double a, double b)
    {
        return std::min<double>(std::fabs(a - b), std::min<double>(std::fabs(a - b + 2*M_PI), std::fabs(a - b - 2*M_PI))); 
    }
}

void ObstacleSolver::add(double direction, double width)
{
    obstacles.push_back(Obstacle(direction, width));
}

void ObstacleSolver::add(const BWAPI::Position& relPosition, BWAPI::UnitType unitType)
{
    double distance = relPosition.getLength();
    double width = 0.5 * std::max(unitType.dimensionLeft() + unitType.dimensionRight(), unitType.dimensionUp() + unitType.dimensionDown());
    add(angleOf(relPosition), atan(width/distance));
}

void ObstacleSolver::add(const BWAPI::Position& tileCenter)
{
    BWAPI::Position dx(16, 0), dy(0, 16);
    BWAPI::Position corners[4] = { tileCenter - dx - dy, tileCenter + dx - dy, tileCenter - dx + dy, tileCenter + dx + dy };
    double direction = angleOf(tileCenter);
    double width = 0.0;
    for (unsigned int k=0; k<4; ++k)
        width = std::max(width, angleDistance(angleOf(corners[k]), direction));
    add(direction, width);
}

BWAPI::Position ObstacleSolver::solve()
{
    if (obstacles.empty())
        return BWAPI::Positions::Unknown;
    double bestDirection   = 0.0;
    double bestWidth       = 0.0;
    std::sort(obstacles.begin(), obstacles.end());
    add(2 * M_PI + obstacles.front().direction, obstacles.front().width);
    //for (auto it : obstacles)
    //    std::cout << "direction: " << it.direction << "; " << it.width << "\n";
    for (unsigned int k=1, size=obstacles.size(); k<size; ++k) {
        auto& lower = obstacles[k-1];
        auto& upper = obstacles[k];
        double width = upper.direction - lower.direction - lower.width - upper.width;
        if (width > bestWidth) {
            bestWidth = width;
            bestDirection = lower.direction + lower.width + 0.5 * width;
        }
    }
    obstacles.clear();
    if (bestWidth <= 0.0)
        return BWAPI::Positions::Unknown;
    double distance = 96.0;
    return BWAPI::Position((int)(distance * cos(bestDirection)), (int)(distance * sin(bestDirection)));
}

void GuerillaExpert::retreat(const std::vector<OwnUnitBoundaryItem*>& ownUnits, const std::vector<EnemyUnitBoundaryItem*>& enemyUnits)
{
    ObstacleSolver solver;
    for (auto unit : ownUnits) {
        BWAPI::Position basePosition = unit->getPosition();
        for (auto enemy : enemyUnits)
            solver.add(enemy->getPosition() - basePosition, enemy->getUnitType());

        BWAPI::TilePosition baseTile(basePosition);
        auto& fields = currentBlackboard->getInformations()->fields;
        for (int x=-3; x<=3; ++x)
            for (int y=-3; y<=3; ++y)
                if ((x != 0) && (y != 0))
        {
            BWAPI::TilePosition tile = baseTile + BWAPI::TilePosition(x, y);
            if (!fields.isValid(tile) || !fields[tile].isMovable())
                solver.add(BWAPI::Position(tile) - basePosition);
        }

        BWAPI::Position dir = solver.solve();
        if (dir != BWAPI::Positions::Unknown)
            retreatTo(unit, basePosition + dir);
    }
}

void GuerillaExpert::retreatTo(OwnUnitBoundaryItem* ownUnit, BWAPI::Position pos)
{
    auto provider = &ownUnit->provideUnit;
    if (provider->isConnected()) {
        auto planItem = dynamic_cast<AbstractSimpleUnitPlanItem*>(provider->getConnectedPort()->getOwner());
        if (planItem == NULL)
            return;
        auto moveItem = dynamic_cast<MoveToPositionPlanItem*>(planItem);
        if ((moveItem != NULL) && (moveItem->getPosition().getDistance(pos) < 32.0))
            return;
        if (provider->isActiveConnection()) {
            provider = &planItem->provideUnit;
            if (provider->isActiveConnection())
                return;
            currentBlackboard->terminate(planItem);
            // ToDo: Copy active plan item (if not created by GuerillaExpert)...
        }
        if (provider->isConnected()) {
            auto nextItem = dynamic_cast<AbstractSimpleUnitPlanItem*>(provider->getConnectedPort()->getOwner());
            if ((nextItem != NULL) && (nextItem->creator == this))
                currentBlackboard->removeItem(nextItem);
        }
    }

    auto nextRequirePort = provider->getConnectedPort();
    auto newPlanItem = currentBlackboard->move(provider, pos);
    newPlanItem->provideUnit.connectTo(nextRequirePort);
}

void GuerillaExpert::cleanup(const std::vector<OwnUnitBoundaryItem*>& ownUnits)
{
    for (auto it : ownUnits) {
        auto provider = &it->provideUnit;
        if (!provider->isConnected())
            continue;
        auto planItem = dynamic_cast<AbstractPlanItem*>(provider->getConnectedPort()->getOwner());
        if ((planItem == NULL) || (planItem->creator != this))
            continue;
        if (planItem->isActive()) {
            currentBlackboard->terminate(planItem);
        } else {
            currentBlackboard->removeItem(planItem);
        }
    }
}
