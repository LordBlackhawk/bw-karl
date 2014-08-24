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

SimpleUnitUpdateEvent::SimpleUnitUpdateEvent(BWAPI::Unit* u, BWAPI::Position p)
    : UnitUpdateEvent(u), pos(p)
{ }

void SimpleUnitUpdateEvent::acceptVisitor(AbstractEventVisitor* visitor)
{
    visitor->visitSimpleUnitUpdateEvent(this);
}

CompleteUnitUpdateEvent::CompleteUnitUpdateEvent(BWAPI::Unit* u, BWAPI::UnitType t, const BWAPI::TilePosition& tp, const BWAPI::Position& p, BWAPI::Player* o)
    : SimpleUnitUpdateEvent(u, p), unitType(t), owner(o), tilePos(tp)
{ }

void CompleteUnitUpdateEvent::acceptVisitor(AbstractEventVisitor* visitor)
{
    visitor->visitCompleteUnitUpdateEvent(this);
}

MineralUpdateEvent::MineralUpdateEvent(BWAPI::Unit* u, int m)
    : UnitUpdateEvent(u), minerals(m)
{ }

void MineralUpdateEvent::acceptVisitor(AbstractEventVisitor* visitor)
{
    visitor->visitMineralUpdateEvent(this);
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
