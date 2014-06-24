#include "basic-expert.hpp"
#include "plan/broodwar-ports.hpp"
#include "plan/broodwar-plan-items.hpp"

BasicExpert::BasicExpert()
{ }

bool BasicExpert::isApplicable(Blackboard* /*blackboard*/)
{
    return true;
}

void BasicExpert::prepare()
{ }

bool BasicExpert::tick(Blackboard* blackboard)
{
    currentBlackboard = blackboard;
    beginTraversal();
    std::vector<AbstractPlanItem*> items = currentBlackboard->getItems();
    for (auto it : items)
        it->acceptVisitor(this);
    endTraversal();
    currentBlackboard = NULL;
    return true;
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

void BasicExpert::visitResourcePort(ResourcePort* port)
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

BasicPortExpert::BasicPortExpert()
    : currentPlanItem(NULL)
{ }

void BasicPortExpert::visitAbstractPlanItem(AbstractPlanItem* item)
{
    currentPlanItem = item;
    if (item->isActive()) {
        // Do not visit active connection to avoid mistakes.
        for (auto it : item->ports)
            if (!it->isActiveConnection() && !it->isRequirePort())
                it->acceptVisitor(this);
    } else {
        for (auto it : item->ports)
            it->acceptVisitor(this);
    }
    currentPlanItem = NULL;
}
