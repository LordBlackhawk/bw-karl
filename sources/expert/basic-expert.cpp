#include "basic-expert.hpp"
#include "plan/broodwar-ports.hpp"
#include "plan/broodwar-plan-items.hpp"

BasicExpert::BasicExpert()
{ }

void BasicExpert::tick(Blackboard* blackboard)
{
    currentBlackboard = blackboard;
    prepare();
    std::vector<AbstractPlanItem*> items = currentBlackboard->getItems();
    for (auto it : items)
        it->acceptVisitor(this);
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

void BasicExpert::prepare()
{ }

void BasicPortExpert::visitAbstractPlanItem(AbstractPlanItem* item)
{
    for (auto it : item->ports)
        it->acceptVisitor(this);
}
