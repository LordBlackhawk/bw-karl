#include "broodwar-boundary-items.hpp"
#include "broodwar-ports.hpp"
#include "abstract-visitor.hpp"
#include "engine/broodwar-events.hpp"

AbstractSpaceUnitBoundaryItem::AbstractSpaceUnitBoundaryItem(BWAPI::Unit* u, Array2d<FieldInformations>* f, BWAPI::UnitType ut)
    : AbstractBoundaryItem(u),
      requireSpace(this, f, ut),
      unitType(ut)
{
    removePort(&requireSpace);
}

void AbstractSpaceUnitBoundaryItem::visitCompleteUnitUpdateEvent(CompleteUnitUpdateEvent* event)
{
    unitType = event->unitType;
    if (unitType.isBuilding()) {
        requireSpace.setUnitType(unitType);
        requireSpace.connectTo(event->tilePos);
        if (!isPortRegistered(&requireSpace))
            ports.push_back(&requireSpace);
    } else {
        requireSpace.disconnect();
        removePort(&requireSpace);
    }
    AbstractBoundaryItem::visitCompleteUnitUpdateEvent(event);
}

int AbstractSpaceUnitBoundaryItem::getHealth() const
{
    return unitType.maxHitPoints() + unitType.maxShields();
}

double AbstractSpaceUnitBoundaryItem::getGroundDPS() const
{
    auto weapon = unitType.groundWeapon();
    return (double)weapon.damageAmount() / (double)weapon.damageCooldown() * 24.0;
}


OwnUnitBoundaryItem::OwnUnitBoundaryItem(BWAPI::Unit* u, BWAPI::UnitType ut, Array2d<FieldInformations>* f)
    : AbstractSpaceUnitBoundaryItem(u, f, ut),
      provideUnit(this),
      supply(this, ut)
{
    provideUnit.estimatedTime = ACTIVE_TIME;
    supply.estimatedTime = ACTIVE_TIME;
}

void OwnUnitBoundaryItem::acceptVisitor(AbstractVisitor* visitor)
{
    visitor->visitOwnUnitBoundaryItem(this);
}

void OwnUnitBoundaryItem::visitCompleteUnitUpdateEvent(CompleteUnitUpdateEvent* event)
{
    if (getUnitType() != event->unitType)
        supply.updateUnitType(event->unitType);
    AbstractSpaceUnitBoundaryItem::visitCompleteUnitUpdateEvent(event);
    provideUnit.updateData(event->unitType, event->pos);
}

void OwnUnitBoundaryItem::visitSimpleUnitUpdateEvent(SimpleUnitUpdateEvent* event)
{
    provideUnit.updateData(getUnitType(), event->pos);
}


ResourceBoundaryItem::ResourceBoundaryItem(BWAPI::Unit* u, BWAPI::UnitType ut, BlackboardInformations* i, BaseLocation* b)
    : AbstractSpaceUnitBoundaryItem(u, &i->fields, ut),
      info(i),
      lastSeen(-100),
      base(b),
      minerals(-1)
{ }

ResourceBoundaryItem::~ResourceBoundaryItem()
{
    if (base != NULL)
        base->minerals.erase(this);
}

void ResourceBoundaryItem::acceptVisitor(AbstractVisitor* visitor)
{
    visitor->visitResourceBoundaryItem(this);
}

void ResourceBoundaryItem::visitMineralUpdateEvent(MineralUpdateEvent* event)
{
    lastSeen = info->lastUpdateTime;
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


EnemyUnitBoundaryItem::EnemyUnitBoundaryItem(BWAPI::Unit* u, BWAPI::UnitType ut, BlackboardInformations* i)
    : AbstractSpaceUnitBoundaryItem(u, &i->fields, ut),
      info(i),
      lastSeen(-100),
      position(BWAPI::Positions::Unknown)
{ }

void EnemyUnitBoundaryItem::acceptVisitor(AbstractVisitor* visitor)
{
    visitor->visitEnemyUnitBoundaryItem(this);
}

void EnemyUnitBoundaryItem::visitSimpleUnitUpdateEvent(SimpleUnitUpdateEvent* event)
{
    lastSeen = info->lastUpdateTime;
    position = event->pos;
}
