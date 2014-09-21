#include "test-smart-turn-around-expert.hpp"
#include "expert-registrar.hpp"
#include "plan/broodwar-boundary-items.hpp"

REGISTER_EXPERT_EX(TestSmartTurnAroundExpert, false)

void TestSmartTurnAroundExpert::visitOwnUnitBoundaryItem(OwnUnitBoundaryItem* item)
{
    if (item->provideUnit.isConnected() || item->isBuilding())
        return;

    BWAPI::Position pos = item->getPosition();
    int rx = pos.x() / 256, ry = pos.y() / 256;
    BWAPI::Position left(256*rx + 64, 256*ry + 128), right(256*rx + 192, 256*ry + 128);
    bool useSmartTurnAround = (ry % 2 == 0);

    
    auto move = currentBlackboard->move(&item->provideUnit, left);
    move->useSmartTurnAround = useSmartTurnAround;
    move = currentBlackboard->move(&move->provideUnit, right);
    move->useSmartTurnAround = useSmartTurnAround;
}
