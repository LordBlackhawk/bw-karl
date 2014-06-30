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

UnitUpdateEvent::UnitUpdateEvent(BWAPI::Unit* u, BWAPI::UnitType t, BWAPI::Position p)
    : unit(u), unitType(t), pos(p)
{ }

void UnitUpdateEvent::acceptVisitor(AbstractEventVisitor* visitor)
{
    visitor->visitUnitUpdateEvent(this);
}

UnitCreateEvent::UnitCreateEvent(BWAPI::Unit* u, BWAPI::UnitType t, BWAPI::Position p, BWAPI::Player* o)
    : UnitUpdateEvent(u, t, p), owner(o)
{ }

void UnitCreateEvent::acceptVisitor(AbstractEventVisitor* visitor)
{
    visitor->visitUnitCreateEvent(this);
}
