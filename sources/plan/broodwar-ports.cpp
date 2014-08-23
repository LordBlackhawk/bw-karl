#include "broodwar-ports.hpp"
#include "abstract-visitor.hpp"
#include "broodwar-boundary-items.hpp"
#include "blackboard-informations.hpp"
#include "engine/basic-actions.hpp"

ProvideUnitPort::ProvideUnitPort(AbstractItem* o, BWAPI::Unit* u, bool od)
    : BaseClass(o), unit(u), unitType(BWAPI::UnitTypes::Unknown), pos(BWAPI::Positions::Unknown), onDemand(od), previousAction(NULL)
{ }

void ProvideUnitPort::updateData(BWAPI::UnitType ut, BWAPI::Position p)
{
    unitType    = ut;
    pos         = p;
}

void ProvideUnitPort::updateData(RequireUnitPort* port)
{
    unit        = port->getUnit();
    unitType    = port->getUnitType();
    pos         = port->getPosition();
}

void ProvideUnitPort::acceptVisitor(AbstractVisitor* visitor)
{
    visitor->visitProvideUnitPort(this);
}

AbstractAction* ProvideUnitPort::prepareForExecution(AbstractExecutionEngine* engine)
{
    if (onDemand)
        engine->addAction(new TerminateAction(previousAction, false));
    return previousAction;
}

RequireUnitPort::RequireUnitPort(AbstractItem* o, BWAPI::UnitType ut)
    : BaseClass(o), unitType(ut)
{ }

void RequireUnitPort::acceptVisitor(AbstractVisitor* visitor)
{
    visitor->visitRequireUnitPort(this);
}

void RequireUnitPort::bridge(ProvideUnitPort* port)
{
    connection->connectTo(port->connection);
}

AbstractAction* RequireUnitPort::prepareForExecution(AbstractExecutionEngine* engine)
{
    if (connection == NULL)
        return NULL;

    return connection->prepareForExecution(engine);
}

ResourcePort::ResourcePort(AbstractItem* o, int m, int g)
    : AbstractPort(o), minerals(m), gas(g)
{ }

bool ResourcePort::isRequirePort() const
{
    return true;
}

bool ResourcePort::isActiveConnection() const
{
    return false;
}

void ResourcePort::acceptVisitor(AbstractVisitor* visitor)
{
    visitor->visitResourcePort(this);
}

void ResourcePort::disconnect()
{
    estimatedTime = INFINITE_TIME;
}

void ResourcePort::updateEstimates()
{
    // do nothing, ResourceExpert updates estimates!!!
}

SupplyPort::SupplyPort(AbstractItem* o, BWAPI::UnitType ut, bool checkTwoInOneEgg)
    : AbstractPort(o), estimatedDuration(0), race(ut.getRace()), providedAmount(ut.supplyProvided() - ut.supplyRequired())
{
    if (checkTwoInOneEgg && ut.isTwoUnitsInOneEgg())
        providedAmount *= 2;
    if (providedAmount == 0)
        owner->removePort(this);
}

void SupplyPort::updateUnitType(BWAPI::UnitType ut)
{
    int newAmount = ut.supplyProvided() - ut.supplyRequired();
    if (newAmount != providedAmount) {
        if (providedAmount == 0)
            owner->ports.push_back(this);
        providedAmount = newAmount;
        if (providedAmount == 0)
            owner->removePort(this);
    }
}

bool SupplyPort::isRequirePort() const
{
    return (providedAmount < 0);
}

bool SupplyPort::isActiveConnection() const
{
    return false;
}

void SupplyPort::acceptVisitor(AbstractVisitor* visitor)
{
    visitor->visitSupplyPort(this);
}

void SupplyPort::disconnect()
{
    estimatedTime = INFINITE_TIME;
}

void SupplyPort::updateEstimates()
{
    if (isRequirePort()) {
        // do nothing, require ports are updated by SupplyExpert!!!
    } else {
        // Remark this method is only called if owner is of type AbstractPlanItem!!!
        estimatedTime = static_cast<AbstractPlanItem*>(owner)->estimatedStartTime + estimatedDuration;
    }
}

ProvideMineralFieldPort::ProvideMineralFieldPort(ResourceBoundaryItem* o)
    : BaseClass(o)
{
    estimatedTime = ACTIVE_TIME;
}

void ProvideMineralFieldPort::acceptVisitor(AbstractVisitor* visitor)
{
    visitor->visitProvideMineralFieldPort(this);
}

BWAPI::Unit* ProvideMineralFieldPort::getUnit() const
{
    return getOwner()->getUnit();
}

ResourceBoundaryItem* ProvideMineralFieldPort::getOwner() const
{
    return static_cast<ResourceBoundaryItem*>(owner);
}

RequireMineralFieldPort::RequireMineralFieldPort(AbstractItem* o, ResourceBoundaryItem* m)
    : BaseClass(o)
{
    connectTo(new ProvideMineralFieldPort(m));
}

void RequireMineralFieldPort::acceptVisitor(AbstractVisitor* visitor)
{
    visitor->visitRequireMineralFieldPort(this);
}

RequireSpacePort::RequireSpacePort(AbstractItem* o, Array2d<FieldInformations>* f, BWAPI::UnitType ut, BWAPI::TilePosition p)
    : AbstractPort(o), fields(f), pos(BWAPI::TilePositions::Unknown), unitType(ut)
{
    connectTo(p);
}

RequireSpacePort::~RequireSpacePort()
{
    disconnect();
}

bool RequireSpacePort::isRequirePort() const
{
    return true;
}

bool RequireSpacePort::isActiveConnection() const
{
    return isActive();
}

void RequireSpacePort::acceptVisitor(AbstractVisitor* visitor)
{
    visitor->visitRequireSpacePort(this);
}

void RequireSpacePort::updateEstimates()
{
    estimatedTime = (isConnected()) ? ACTIVE_TIME : INFINITE_TIME;
}

void RequireSpacePort::disconnect()
{
    if (isConnected()) {
        for (int x=pos.x(); x<pos.x()+getWidth(); ++x)
            for (int y=pos.y(); y<pos.y()+getHeight(); ++y)
                (*fields)[x][y].blocker = NULL;
        pos = BWAPI::TilePositions::Unknown;
    }
}

void RequireSpacePort::setUnitType(BWAPI::UnitType ut)
{
    BWAPI::TilePosition oldpos = pos;
    disconnect();
    unitType = ut;
    connectTo(oldpos);
}

void RequireSpacePort::connectTo(BWAPI::TilePosition tp)
{
    if (pos == tp)
        return;

    if (isConnected())
        disconnect();
    pos = tp;
    if (isConnected()) {
        for (int x=pos.x(); x<pos.x()+getWidth(); ++x)
            for (int y=pos.y(); y<pos.y()+getHeight(); ++y)
        {
            auto& field = (*fields)[x][y];
            if (field.blocker != NULL)
                field.blocker->disconnect();
            field.blocker = this;
        }
    }
}


// EnemyUnit
ProvideEnemyUnitPort::ProvideEnemyUnitPort(EnemyUnitBoundaryItem* o)
    : BaseClass(o)
{
    estimatedTime = ACTIVE_TIME;
}

void ProvideEnemyUnitPort::acceptVisitor(AbstractVisitor* visitor)
{
    visitor->visitProvideEnemyUnitPort(this);
}

BWAPI::Unit* ProvideEnemyUnitPort::getUnit() const
{
    return getOwner()->getUnit();
}

BWAPI::Position ProvideEnemyUnitPort::getPosition() const 
{ 
    return getOwner()->getPosition();
}

EnemyUnitBoundaryItem* ProvideEnemyUnitPort::getOwner() const
{
    return static_cast<EnemyUnitBoundaryItem*>(owner);
}

RequireEnemyUnitPort::RequireEnemyUnitPort(AbstractItem* o, EnemyUnitBoundaryItem* enemy)
    : BaseClass(o)
{
    connectTo(new ProvideEnemyUnitPort(enemy));
}

void RequireEnemyUnitPort::acceptVisitor(AbstractVisitor* visitor)
{
    visitor->visitRequireEnemyUnitPort(this);
}

ProvideUnitExistancePort::ProvideUnitExistancePort(AbstractItem* o, BWAPI::UnitType ut)
    : BaseClass(o), unitType(ut)
{
    if (owner->isPlanItem()) {
        estimatedDuration = ut.buildTime();
    } else if (owner->isBoundaryItem()) {
        estimatedTime = ACTIVE_TIME;
    }
}

void ProvideUnitExistancePort::acceptVisitor(AbstractVisitor* visitor)
{
    visitor->visitProvideUnitExistancePort(this);
}

RequireUnitExistancePort::RequireUnitExistancePort(AbstractItem* o, BWAPI::UnitType ut)
    : BaseClass(o), unitType(ut)
{ }

void RequireUnitExistancePort::acceptVisitor(AbstractVisitor* visitor)
{
    visitor->visitRequireUnitExistancePort(this);
}

void RequireUnitExistancePort::connectTo(AbstractItem* provider)
{
    if (isConnected() && (getConnectedPort()->getOwner() == provider))
        return;
    BaseClass::connectTo(new ProvideUnitExistancePort(provider, unitType));
}
