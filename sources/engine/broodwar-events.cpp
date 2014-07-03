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

OwnUnitUpdateEvent::OwnUnitUpdateEvent(BWAPI::Unit* u, BWAPI::UnitType t, BWAPI::Position p)
    : UnitUpdateEvent(u), unitType(t), pos(p)
{ }

void OwnUnitUpdateEvent::acceptVisitor(AbstractEventVisitor* visitor)
{
    visitor->visitOwnUnitUpdateEvent(this);
}

MineralUpdateEvent::MineralUpdateEvent(BWAPI::Unit* u, int m)
    : UnitUpdateEvent(u), minerals(m)
{ }

void MineralUpdateEvent::acceptVisitor(AbstractEventVisitor* visitor)
{
    visitor->visitMineralUpdateEvent(this);
}

UnitCreateEvent::UnitCreateEvent(BWAPI::Unit* u, BWAPI::UnitType t, const BWAPI::TilePosition& tp, const BWAPI::Position& p, BWAPI::Player* o)
    : unit(u), unitType(t), owner(o), tilePos(tp), pos(p)
{ }

void UnitCreateEvent::acceptVisitor(AbstractEventVisitor* visitor)
{
    visitor->visitUnitCreateEvent(this);
}
