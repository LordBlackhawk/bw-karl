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

ResourceBoundaryItem::ResourceBoundaryItem(BWAPI::Unit* u, BWAPI::UnitType ut, Array2d<FieldInformations>* f, BaseLocation* b)
    : AbstractBoundaryItem(u),
      unitType(ut),
      requireSpace(this, f, ut),
      base(b),
      minerals(-1)
{
    ports.push_back(&requireSpace);
}

ResourceBoundaryItem::~ResourceBoundaryItem()
{
    if (base != NULL)
        base->minerals.erase(this);
    // There maybe ProvideMineralFieldPorts (dynamically created)!
    while (!ports.empty()) {
        auto port = dynamic_cast<ProvideMineralFieldPort*>(ports.front());
        if (port != NULL) {
            port->disconnect();
        } else {
            ports.erase(ports.begin());
        }
    }
}

void ResourceBoundaryItem::acceptVisitor(AbstractVisitor* visitor)
{
    visitor->visitResourceBoundaryItem(this);
}

void ResourceBoundaryItem::visitUnitCreateEvent(UnitCreateEvent* event)
{
    requireSpace.connectTo(event->tilePos);
}

void ResourceBoundaryItem::visitMineralUpdateEvent(MineralUpdateEvent* event)
{
    minerals = event->minerals;
}
