#include "broodwar-events.hpp"
#include "event-visitor.hpp"

FrameEvent::FrameEvent(Time t, int m, int g)
    : currentTime(t), currentMinerals(m), currentGas(g)
{ }

void FrameEvent::acceptVisitor(AbstractEventVisitor* visitor)
{
    visitor->visitFrameEvent(this);
}

BroodwarEvent::BroodwarEvent(const BWAPI::Event& e)
    : event(e)
{ }

void BroodwarEvent::acceptVisitor(AbstractEventVisitor* visitor)
{
    visitor->visitBroodwarEvent(this);
}

UnitUpdateEvent::UnitUpdateEvent(BWAPI::Unit* u)
    : unit(u)
{ }

void UnitUpdateEvent::acceptVisitor(AbstractEventVisitor* visitor)
{
    visitor->visitUnitUpdateEvent(this);
}

SimpleUnitUpdateEvent::SimpleUnitUpdateEvent(BWAPI::Unit* u, BWAPI::Position p, int h, BWAction::Type a)
    : UnitUpdateEvent(u), pos(p), health(h), currentAction(a)
{ }

void SimpleUnitUpdateEvent::acceptVisitor(AbstractEventVisitor* visitor)
{
    visitor->visitSimpleUnitUpdateEvent(this);
}

CompleteUnitUpdateEvent::CompleteUnitUpdateEvent(BWAPI::Unit* u, BWAPI::UnitType t, int h, BWAction::Type a, const BWAPI::TilePosition& tp, const BWAPI::Position& p, BWAPI::Player* o)
    : SimpleUnitUpdateEvent(u, p, h, a), unitType(t), owner(o), tilePos(tp)
{ }

void CompleteUnitUpdateEvent::acceptVisitor(AbstractEventVisitor* visitor)
{
    visitor->visitCompleteUnitUpdateEvent(this);
}

ResourceUpdateEvent::ResourceUpdateEvent(BWAPI::Unit* u, int res)
    : UnitUpdateEvent(u), resources(res)
{ }

void ResourceUpdateEvent::acceptVisitor(AbstractEventVisitor* visitor)
{
    visitor->visitResourceUpdateEvent(this);
}

FieldSeenEvent::FieldSeenEvent(const BWAPI::TilePosition& tp, bool c)
    : tilePos(tp), creep(c)
{ }

void FieldSeenEvent::acceptVisitor(AbstractEventVisitor* visitor)
{
    visitor->visitFieldSeenEvent(this);
}

AbstractActionEvent::AbstractActionEvent(AbstractAction* a)
    : sender(a)
{ }

ResourcesConsumedEvent::ResourcesConsumedEvent(AbstractAction* a)
    : AbstractActionEvent(a)
{ }

void ResourcesConsumedEvent::acceptVisitor(AbstractEventVisitor* visitor)
{
    visitor->visitResourcesConsumedEvent(this);
}
