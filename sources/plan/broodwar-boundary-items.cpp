#include "broodwar-boundary-items.hpp"
#include "abstract-visitor.hpp"
#include "engine/broodwar-events.hpp"

OwnUnitBoundaryItem::OwnUnitBoundaryItem(BWAPI::Unit* u)
    : AbstractBoundaryItem(u), provideUnit(u)
{
    ports.push_back(&provideUnit);
    provideUnit.estimatedTime = ACTIVE_TIME;
}

void OwnUnitBoundaryItem::acceptVisitor(AbstractVisitor* visitor)
{
    visitor->visitOwnUnitBoundaryItem(this);
}

void OwnUnitBoundaryItem::visitOwnUnitUpdateEvent(OwnUnitUpdateEvent* event)
{
    provideUnit.updateData(event->unitType, event->pos);
}

MineralBoundaryItem::MineralBoundaryItem(BWAPI::Unit* u, BaseLocation* b)
    : AbstractBoundaryItem(u), base(b), pos(BWAPI::TilePositions::Unknown), minerals(-1)
{ }

MineralBoundaryItem::~MineralBoundaryItem()
{
    if (base != NULL)
        base->minerals.erase(this);
}

void MineralBoundaryItem::acceptVisitor(AbstractVisitor* visitor)
{
    visitor->visitMineralBoundaryItem(this);
}

void MineralBoundaryItem::visitUnitCreateEvent(UnitCreateEvent* event)
{
    pos = event->tilePos;
}

void MineralBoundaryItem::visitMineralUpdateEvent(MineralUpdateEvent* event)
{
    minerals = event->minerals;
}
