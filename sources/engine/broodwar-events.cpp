#include "broodwar-events.hpp"
#include "event-visitor.hpp"

FrameEvent::FrameEvent(Time t, int m, int g)
    : currentTime(t), currentMinerals(m), currentGas(g)
{ }

void FrameEvent::acceptVisitor(AbstractEventVisitor* visitor)
{
    visitor->visitFrameEvent(this);
}

BroodwarEvent::BroodwarEvent(BWAPI::Event e)
    : event(e)
{ }

void BroodwarEvent::acceptVisitor(AbstractEventVisitor* visitor)
{
    visitor->visitBroodwarEvent(this);
}

UnitEvent::UnitEvent(BWAPI::Unit* u, BWAPI::UnitType t, BWAPI::Position p)
    : unit(u), unitType(t), pos(p)
{ }

void UnitEvent::acceptVisitor(AbstractEventVisitor* visitor)
{
    visitor->visitUnitEvent(this);
}
