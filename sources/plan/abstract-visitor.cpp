#include "abstract-visitor.hpp"
#include "broodwar-ports.hpp"
#include "broodwar-boundary-items.hpp"
#include "broodwar-plan-items.hpp"

void BasicVisitor::visitAbstractPort(AbstractPort* /*port*/)
{ }

void BasicVisitor::visitProvideUnitPort(ProvideUnitPort* port)
{
    visitAbstractPort(port);
}

void BasicVisitor::visitRequireUnitPort(RequireUnitPort* port)
{
    visitAbstractPort(port);
}

void BasicVisitor::visitResourcePort(ResourcePort* port)
{
    visitAbstractPort(port);
}

void BasicVisitor::visitSupplyPort(SupplyPort* port)
{
    visitAbstractPort(port);
}

void BasicVisitor::visitProvideMineralFieldPort(ProvideMineralFieldPort* port)
{
    visitAbstractPort(port);
}

void BasicVisitor::visitRequireMineralFieldPort(RequireMineralFieldPort* port)
{
    visitAbstractPort(port);
}

void BasicVisitor::visitRequireSpacePort(RequireSpacePort* port)
{
    visitAbstractPort(port);
}

void BasicVisitor::visitProvideEnemyUnitPort(ProvideEnemyUnitPort* port)
{
    visitAbstractPort(port);
}

void BasicVisitor::visitRequireEnemyUnitPort(RequireEnemyUnitPort* port)
{
    visitAbstractPort(port);
}

void BasicVisitor::visitProvideUnitExistancePort(ProvideUnitExistancePort* port)
{
    visitAbstractPort(port);
}

void BasicVisitor::visitRequireUnitExistancePort(RequireUnitExistancePort* port)
{
    visitAbstractPort(port);
}


void BasicVisitor::visitAbstractBoundaryItem(AbstractBoundaryItem* /*item*/)
{ }

void BasicVisitor::visitOwnUnitBoundaryItem(OwnUnitBoundaryItem* item)
{
    visitAbstractBoundaryItem(item);
}

void BasicVisitor::visitResourceBoundaryItem(ResourceBoundaryItem* item)
{
    visitAbstractBoundaryItem(item);
}

void BasicVisitor::visitEnemyUnitBoundaryItem(EnemyUnitBoundaryItem* item)
{
    visitAbstractBoundaryItem(item);
}

void BasicVisitor::visitAbstractPlanItem(AbstractPlanItem* /*item*/)
{ }

void BasicVisitor::visitGatherMineralPlanItem(GatherMineralsPlanItem* item)
{
    visitAbstractPlanItem(item);
}

void BasicVisitor::visitMorphUnitPlanItem(MorphUnitPlanItem* item)
{
    visitAbstractPlanItem(item);
}

void BasicVisitor::visitMoveToPositionPlanItem(MoveToPositionPlanItem* item)
{
    visitAbstractPlanItem(item);
}

void BasicVisitor::visitBuildPlanItem(BuildPlanItem* item)
{
    visitAbstractPlanItem(item);
}

void BasicVisitor::visitAttackUnitPlanItem(AttackUnitPlanItem* item)
{
    visitAbstractPlanItem(item);
}

void BasicVisitor::visitAttackPositionPlanItem(AttackPositionPlanItem* item)
{
    visitAbstractPlanItem(item);
}

void BasicVisitor::visitGiveUpPlanItem(GiveUpPlanItem* item)
{
    visitAbstractPlanItem(item);
}
