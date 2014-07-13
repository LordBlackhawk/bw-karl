#include "broodwar-boundary-items.hpp"
#include "broodwar-ports.hpp"
#include "abstract-visitor.hpp"
#include "engine/broodwar-events.hpp"

OwnUnitBoundaryItem::OwnUnitBoundaryItem(BWAPI::Unit* u, Array2d<FieldInformations>* f)
    : AbstractBoundaryItem(u),
      provideUnit(this, u),
      requireSpace(this, f, BWAPI::UnitTypes::Unknown)
{
    ports.push_back(&provideUnit);
    provideUnit.estimatedTime = ACTIVE_TIME;
}

void OwnUnitBoundaryItem::acceptVisitor(AbstractVisitor* visitor)
{
    visitor->visitOwnUnitBoundaryItem(this);
}

void OwnUnitBoundaryItem::visitUnitCreateEvent(UnitCreateEvent* event)
{
    if (event->unitType.isBuilding()) {
        requireSpace.setUnitType(event->unitType);
        requireSpace.connectTo(event->tilePos);
    }
    provideUnit.updateData(event->unitType, event->pos);
}

void OwnUnitBoundaryItem::visitOwnUnitUpdateEvent(OwnUnitUpdateEvent* event)
{
    provideUnit.updateData(event->unitType, event->pos);
}

MineralBoundaryItem::MineralBoundaryItem(BWAPI::Unit* u, Array2d<FieldInformations>* f, BaseLocation* b)
    : AbstractBoundaryItem(u),
      requireSpace(this, f, BWAPI::UnitTypes::Resource_Mineral_Field),
      base(b),
      minerals(-1)
{ }

MineralBoundaryItem::~MineralBoundaryItem()
{
    if (base != NULL)
        base->minerals.erase(this);
    // There maybe ProvideMineralFieldPorts (dynamically created)!
    while (!ports.empty()) {
        auto port = dynamic_cast<ProvideMineralFieldPort*>(ports.front());
        if (port != NULL)
            port->disconnect();
    }
}

void MineralBoundaryItem::acceptVisitor(AbstractVisitor* visitor)
{
    visitor->visitMineralBoundaryItem(this);
}

void MineralBoundaryItem::visitUnitCreateEvent(UnitCreateEvent* event)
{
    requireSpace.connectTo(event->tilePos);
}

void MineralBoundaryItem::visitMineralUpdateEvent(MineralUpdateEvent* event)
{
    minerals = event->minerals;
}
