#include "clean-up-expert.hpp"
#include "expert-registrar.hpp"
#include "plan/broodwar-plan-items.hpp"
#include <algorithm>

REGISTER_EXPERT(CleanUpExpert)

void CleanUpExpert::checkPorts(AbstractPlanItem* item)
{
    auto& ports = item->ports;
    auto it = std::find_if(ports.begin(), ports.end(), [] (AbstractPort* port) {
            auto purposePort = dynamic_cast<RequirePurposePort*>(port);
            return (purposePort != NULL) && !purposePort->isConnected();
        });
    if (it != ports.end()) {
        currentBlackboard->removeItem(item);
        return;
    }
}

void CleanUpExpert::visitAbstractPlanItem(AbstractPlanItem* item)
{
    if (item->isFailed()) {
        currentBlackboard->removeItem(item);
        return;
    }

    checkPorts(item);
}

void CleanUpExpert::visitBuildPlanItem(BuildPlanItem* item)
{
    // Do not check for failed, BuildingPlacementExpert is responsible.
    checkPorts(item);
}

void CleanUpExpert::visitMoveToPositionPlanItem(MoveToPositionPlanItem* item)
{
    if (!item->isActive() && !item->requireUnit.isConnected()) {
        currentBlackboard->removeItem(item);
        return;
    }
    BasicExpert::visitMoveToPositionPlanItem(item);
}

void CleanUpExpert::visitAttackUnitPlanItem(AttackUnitPlanItem* item)
{
    if (!item->isActive() && (!item->requireUnit.isConnected() || !item->enemyUnit.isConnected())) {
        currentBlackboard->removeItem(item);
        return;
    }
    BasicExpert::visitAttackUnitPlanItem(item);
}

void CleanUpExpert::visitAttackPositionPlanItem(AttackPositionPlanItem* item)
{
    if (!item->isActive() && !item->requireUnit.isConnected()) {
        currentBlackboard->removeItem(item);
        return;
    }
    BasicExpert::visitAttackPositionPlanItem(item);
}
