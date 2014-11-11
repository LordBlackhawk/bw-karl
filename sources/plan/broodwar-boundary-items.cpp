#include "broodwar-boundary-items.hpp"
#include "broodwar-ports.hpp"
#include "abstract-visitor.hpp"
#include "engine/broodwar-events.hpp"

AbstractSpaceUnitBoundaryItem::AbstractSpaceUnitBoundaryItem(BWAPI::Unit* u, BlackboardInformations* i, BWAPI::UnitType ut)
    : AbstractBoundaryItem(u),
      requireSpace(this, i, ut),
      unitType(ut),
      info(i)
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

int AbstractSpaceUnitBoundaryItem::getMaxHealth() const
{
    return unitType.maxHitPoints() + unitType.maxShields();
}

double AbstractSpaceUnitBoundaryItem::getGroundDPS() const
{
    auto weapon = unitType.groundWeapon();
    return (double)weapon.damageAmount() / (double)weapon.damageCooldown() * 24.0;
}

void AbstractSpaceUnitBoundaryItem::takeConnectionsFrom(AbstractBoundaryItem* other)
{
    auto otherCasted = dynamic_cast<AbstractSpaceUnitBoundaryItem*>(other);
    if (otherCasted != NULL)
        requireSpace.connectTo(otherCasted->requireSpace.getTilePosition());
    AbstractBoundaryItem::takeConnectionsFrom(other);
}


OwnUnitBoundaryItem::OwnUnitBoundaryItem(BWAPI::Unit* u, BWAPI::UnitType ut, BlackboardInformations* i)
    : AbstractSpaceUnitBoundaryItem(u, i, ut),
      provideUnit(this),
      supply(this, ut),
      health(getMaxHealth())
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
    health = event->health;
}

void OwnUnitBoundaryItem::takeConnectionsFrom(AbstractBoundaryItem* other)
{
    auto otherCasted = dynamic_cast<OwnUnitBoundaryItem*>(other);
    if (otherCasted != NULL)
        provideUnit.connectTo(otherCasted->provideUnit.getConnectedPort());
    AbstractSpaceUnitBoundaryItem::takeConnectionsFrom(other);
}


namespace
{
    const Time larvaSpawnTime = 300;
}

OwnHatcheryBoundaryItem::OwnHatcheryBoundaryItem(BWAPI::Unit* u, BWAPI::UnitType ut, BlackboardInformations* i)
    : OwnUnitBoundaryItem(u, ut, i)
{ }

OwnHatcheryBoundaryItem::~OwnHatcheryBoundaryItem()
{
    for (auto it : larvas)
        delete it;
}

void OwnHatcheryBoundaryItem::acceptVisitor(AbstractVisitor* visitor)
{
    visitor->visitOwnHatcheryBoundaryItem(this);
}

void OwnHatcheryBoundaryItem::visitSimpleUnitUpdateEvent(SimpleUnitUpdateEvent* event)
{
    OwnUnitBoundaryItem::visitSimpleUnitUpdateEvent(event);
    for (unsigned int k=0, size=larvas.size(); k<size; ++k)
        larvas[k]->provideUnit.estimatedTime = info->lastUpdateTime + k * larvaSpawnTime;
}

Time OwnHatcheryBoundaryItem::lastPlanedLarva() const
{
    if (larvas.empty())
        return info->lastUpdateTime;

    return larvas.back()->provideUnit.estimatedTime;
}

ProvideUnitPort* OwnHatcheryBoundaryItem::createNewLarva()
{
    OwnUnitBoundaryItem* result = new OwnUnitBoundaryItem(NULL, BWAPI::UnitTypes::Zerg_Larva, info);
    result->provideUnit.updateData(BWAPI::UnitTypes::Zerg_Larva, getPosition());
    result->provideUnit.estimatedTime = lastPlanedLarva() + larvaSpawnTime;
    larvas.push_back(result);
    return &result->provideUnit;
}

OwnUnitBoundaryItem* OwnHatcheryBoundaryItem::removeFirstPlanedLarva()
{
    if (larvas.empty())
        return NULL;
    OwnUnitBoundaryItem* result = larvas.front();
    larvas.erase(larvas.begin());
    result->provideUnit.estimatedTime = 0;
    return result;
}


ResourceBoundaryItem::ResourceBoundaryItem(BWAPI::Unit* u, BWAPI::UnitType ut, BlackboardInformations* i, BaseLocation* b)
    : AbstractSpaceUnitBoundaryItem(u, i, ut),
      info(i),
      lastSeen(-100),
      base(b),
      minerals(-1),
      gas(-1)
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

void ResourceBoundaryItem::visitResourceUpdateEvent(ResourceUpdateEvent* event)
{
    lastSeen = info->lastUpdateTime;

    if(getUnitType().isMineralField())
        minerals = event->resources;
    else
    {
        gas = event->resources;
    }
}

int ResourceBoundaryItem::numberOfWorkers() const
{
    int result = 0;
    for (auto port : ports)
        if (dynamic_cast<ProvideResourcePort*>(port) != NULL)
            ++result;
    return result;
}


EnemyUnitBoundaryItem::EnemyUnitBoundaryItem(BWAPI::Unit* u, BWAPI::UnitType ut, BlackboardInformations* i)
    : AbstractSpaceUnitBoundaryItem(u, i, ut),
      info(i),
      lastSeen(-100),
      position(BWAPI::Positions::Unknown),
      health(getMaxHealth()),
      currentAction(BWAction::Unknown)
{ }

void EnemyUnitBoundaryItem::acceptVisitor(AbstractVisitor* visitor)
{
    visitor->visitEnemyUnitBoundaryItem(this);
}

void EnemyUnitBoundaryItem::visitSimpleUnitUpdateEvent(SimpleUnitUpdateEvent* event)
{
    lastSeen = info->lastUpdateTime;
    position = event->pos;
    health = event->health;
    currentAction = event->currentAction;
}
