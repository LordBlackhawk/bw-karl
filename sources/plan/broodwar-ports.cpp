#include "broodwar-ports.hpp"
#include "abstract-visitor.hpp"
#include "broodwar-boundary-items.hpp"
#include "engine/basic-actions.hpp"

ProvideUnitPort::ProvideUnitPort(BWAPI::Unit* u, bool od)
    : unit(u), unitType(BWAPI::UnitTypes::Unknown), pos(BWAPI::Positions::Unknown), onDemand(od), previousAction(NULL)
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

RequireUnitPort::RequireUnitPort(BWAPI::UnitType ut)
    : unitType(ut)
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

ResourcePort::ResourcePort(int m, int g)
    : minerals(m), gas(g)
{ }

bool ResourcePort::isRequirePort() const
{
    return true;
}

bool ResourcePort::isActiveConnection() const
{
    return isActive();
}

void ResourcePort::acceptVisitor(AbstractVisitor* visitor)
{
    visitor->visitResourcePort(this);
}

ProvideMineralFieldPort::ProvideMineralFieldPort(MineralBoundaryItem* o)
    : owner(o)
{
    estimatedTime = ACTIVE_TIME;
}

void ProvideMineralFieldPort::disconnect()
{
    if (isConnected()) {
        BasicPortImpl<ProvideMineralFieldPort, RequireMineralFieldPort, false>::disconnect();
        owner->removePort(this);
        delete this;
    }
}

void ProvideMineralFieldPort::acceptVisitor(AbstractVisitor* visitor)
{
    visitor->visitProvideMineralFieldPort(this);
}

BWAPI::Unit* ProvideMineralFieldPort::getUnit() const
{
    return owner->getUnit();
}

RequireMineralFieldPort::RequireMineralFieldPort(MineralBoundaryItem* o)
{
    if (o != NULL) {
        auto provider = new ProvideMineralFieldPort(o);
        o->ports.push_back(provider);
        connectTo(provider);
    }
}

void RequireMineralFieldPort::acceptVisitor(AbstractVisitor* visitor)
{
    visitor->visitRequireMineralFieldPort(this);
}
