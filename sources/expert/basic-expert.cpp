#include "basic-expert.hpp"
#include "webgui-expert.hpp"
#include "plan/broodwar-ports.hpp"
#include "plan/broodwar-plan-items.hpp"

BasicExpert::BasicExpert()
    : currentBlackboard(NULL)
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
    for (auto it : currentBlackboard->getBoundaries())
        it.second->acceptVisitor(this);
    std::vector<AbstractPlanItem*> items = currentBlackboard->getItems();
    for (auto it : items)
        it->acceptVisitor(this);
    endTraversal();
    currentBlackboard = NULL;
    return true;
}

void BasicExpert::matchEnd(Blackboard */*blackboard*/)
{ }

void BasicExpert::beginTraversal()
{ }

void BasicExpert::endTraversal()
{ }

void BasicExpert::interrupt()
{
    if(currentBlackboard==NULL)
        std::cout<<"BasicExpert::interrupt() unable to interrupt since we do not have a blackboard!";
    
    if ((currentBlackboard != NULL) && (WebGUIExpert::instance() != NULL))
        WebGUIExpert::interruptEngineExecution(currentBlackboard);
}

BasicPortExpert::BasicPortExpert()
    : currentItem(NULL)
{ }

void BasicPortExpert::visitAbstractBoundaryItem(AbstractBoundaryItem* item)
{
    currentItem = item;
    for (auto it : item->ports)
        if (!it->isActiveConnection())
            it->acceptVisitor(this);
    currentItem = NULL;
}

void BasicPortExpert::visitAbstractPlanItem(AbstractPlanItem* item)
{
    currentItem = item;
    if (item->isActive()) {
        // Do not visit active connection to avoid mistakes.
        for (auto it : item->ports)
            if (!it->isActiveConnection())
                it->acceptVisitor(this);
    } else {
        for (auto it : item->ports)
            it->acceptVisitor(this);
    }
    currentItem = NULL;
}
