#include "broodwar-boundary-items.hpp"
#include "broodwar-ports.hpp"
#include "abstract-visitor.hpp"
#include "engine/broodwar-events.hpp"

AbstractSpaceUnitBoundaryItem::AbstractSpaceUnitBoundaryItem(BWAPI::Unit* u, Array2d<FieldInformations>* f, BWAPI::UnitType ut)
    : AbstractBoundaryItem(u),
      requireSpace(this, f, ut),
      unitType(ut)
{ }

void AbstractSpaceUnitBoundaryItem::visitCompleteUnitUpdateEvent(CompleteUnitUpdateEvent* event)
{
    unitType = event->unitType;
    if (unitType.isBuilding()) {
        requireSpace.setUnitType(unitType);
        requireSpace.connectTo(event->tilePos);
        ports.push_back(&requireSpace);
    } else {
        requireSpace.disconnect();
        removePort(&requireSpace);
    }
}


OwnUnitBoundaryItem::OwnUnitBoundaryItem(BWAPI::Unit* u, Array2d<FieldInformations>* f)
    : AbstractSpaceUnitBoundaryItem(u, f),
      provideUnit(this, u)
{
    ports.push_back(&provideUnit);
    provideUnit.estimatedTime = ACTIVE_TIME;
}

void OwnUnitBoundaryItem::acceptVisitor(AbstractVisitor* visitor)
{
    visitor->visitOwnUnitBoundaryItem(this);
}

void OwnUnitBoundaryItem::visitCompleteUnitUpdateEvent(CompleteUnitUpdateEvent* event)
{
    AbstractSpaceUnitBoundaryItem::visitCompleteUnitUpdateEvent(event);
    provideUnit.updateData(event->unitType, event->pos);
}

void OwnUnitBoundaryItem::visitSimpleUnitUpdateEvent(SimpleUnitUpdateEvent* event)
{
    provideUnit.updateData(getUnitType(), event->pos);
}


ResourceBoundaryItem::ResourceBoundaryItem(BWAPI::Unit* u, BWAPI::UnitType ut, Array2d<FieldInformations>* f, BaseLocation* b)
    : AbstractSpaceUnitBoundaryItem(u, f, ut),
      base(b),
      minerals(-1)
{ }

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

void ResourceBoundaryItem::visitMineralUpdateEvent(MineralUpdateEvent* event)
{
    minerals = event->minerals;
}

int ResourceBoundaryItem::numberOfWorkers() const
{
    int result = 0;
    for (auto port : ports)
        if (dynamic_cast<ProvideMineralFieldPort*>(port) != NULL)
            ++result;
    return result;
}


EnemyUnitBoundaryItem::EnemyUnitBoundaryItem(BWAPI::Unit* u, BWAPI::UnitType ut, Array2d<FieldInformations>* f)
    : AbstractSpaceUnitBoundaryItem(u, f, ut),
      position(BWAPI::Positions::Unknown)
{ }

void EnemyUnitBoundaryItem::acceptVisitor(AbstractVisitor* visitor)
{
    visitor->visitEnemyUnitBoundaryItem(this);
}

void EnemyUnitBoundaryItem::visitSimpleUnitUpdateEvent(SimpleUnitUpdateEvent* event)
{
    position = event->pos;
}
