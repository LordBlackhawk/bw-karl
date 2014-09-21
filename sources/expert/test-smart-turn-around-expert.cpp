#include "test-smart-turn-around-expert.hpp"
#include "expert-registrar.hpp"
#include "plan/broodwar-boundary-items.hpp"

REGISTER_EXPERT_EX(TestSmartTurnAroundExpert, false)

void TestSmartTurnAroundExpert::visitOwnUnitBoundaryItem(OwnUnitBoundaryItem* item)
{
    auto port = &item->provideUnit;
    if (port->isConnected())
        return;

    BWAPI::Position pos = item->getPosition();
    int rx = pos.x() / 256, ry = pos.y() / 256;
    BWAPI::Position left(256*rx + 64, 256*ry + 128), right(256*rx + 192, 256*ry + 128);

    port = &(currentBlackboard->move(port, left))->provideUnit;
    currentBlackboard->move(port, right);
}
