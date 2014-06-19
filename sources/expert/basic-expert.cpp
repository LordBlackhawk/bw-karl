#include "basic-expert.hpp"
#include "plan/broodwar-ports.hpp"
#include "plan/broodwar-plan-items.hpp"

BasicExpert::BasicExpert()
{ }

void BasicExpert::prepare()
{ }

void BasicExpert::tick(Blackboard* blackboard)
{
    currentBlackboard = blackboard;
    beginTraversal();
    std::vector<AbstractPlanItem*> items = currentBlackboard->getItems();
    for (auto it : items)
        it->acceptVisitor(this);
    endTraversal();
    currentBlackboard = NULL;
}

void BasicExpert::visitAbstractPort(AbstractPort* /*port*/)
{ }

void BasicExpert::visitProvideUnitPort(ProvideUnitPort* port)
{
    visitAbstractPort(port);
}

void BasicExpert::visitRequireUnitPort(RequireUnitPort* port)
{
    visitAbstractPort(port);
}

void BasicExpert::visitAbstractPlanItem(AbstractPlanItem* /*item*/)
{ }

void BasicExpert::visitOwnUnitPlanItem(OwnUnitPlanItem* item)
{
    visitAbstractPlanItem(item);
}

void BasicExpert::visitGatherMineralPlanItem(GatherMineralsPlanItem* item)
{
    visitAbstractPlanItem(item);
}

void BasicExpert::visitBuildPlanItem(BuildPlanItem* item)
{
    visitAbstractPlanItem(item);
}

void BasicExpert::beginTraversal()
{ }

void BasicExpert::endTraversal()
{ }

void BasicPortExpert::visitAbstractPlanItem(AbstractPlanItem* item)
{
    for (auto it : item->ports)
        it->acceptVisitor(this);
}
