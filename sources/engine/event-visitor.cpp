#include "event-visitor.hpp"
#include "broodwar-events.hpp"

void BasicEventVisitor::visitActionEvent(ActionEvent* /*event*/)
{ }

void BasicEventVisitor::visitFrameEvent(FrameEvent* /*event*/)
{ }

void BasicEventVisitor::visitBroodwarEvent(BroodwarEvent* /*event*/)
{ }

void BasicEventVisitor::visitUnitUpdateEvent(UnitUpdateEvent* /*event*/)
{ }

void BasicEventVisitor::visitSimpleUnitUpdateEvent(SimpleUnitUpdateEvent* event)
{
    visitUnitUpdateEvent(event);
}

void BasicEventVisitor::visitMineralUpdateEvent(MineralUpdateEvent* event)
{
    visitUnitUpdateEvent(event);
}

void BasicEventVisitor::visitCompleteUnitUpdateEvent(CompleteUnitUpdateEvent* /*event*/)
{ }
