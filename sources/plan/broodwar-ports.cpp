#include "broodwar-ports.hpp"
#include "abstract-visitor.hpp"
#include "engine/basic-actions.hpp"

ProvideUnitPort::ProvideUnitPort(BWAPI::Unit* u, bool od)
    : connection(NULL), unit(u), unitType(BWAPI::UnitTypes::Unknown), pos(BWAPI::Positions::Unknown), onDemand(od), previousAction(NULL)
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

bool ProvideUnitPort::isRequirePort() const
{
    return false;
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

void ProvideUnitPort::connectTo(RequireUnitPort* port)
{
    disconnect();
    if (port != NULL)
        port->connectTo(this);
}

void ProvideUnitPort::disconnect()
{
    if (connection != NULL)
        connection->disconnect();
}

RequireUnitPort::RequireUnitPort(BWAPI::UnitType ut)
    : connection(NULL), unitType(ut)
{ }

bool RequireUnitPort::isRequirePort() const
{
    return true;
}

void RequireUnitPort::acceptVisitor(AbstractVisitor* visitor)
{
    visitor->visitRequireUnitPort(this);
}

void RequireUnitPort::updateEstimates()
{
    if (isConnected())
        estimatedTime = connection->estimatedTime;
}

void RequireUnitPort::connectTo(ProvideUnitPort* port)
{
    disconnect();
    if (port != NULL) {
        connection = port;
        port->connection = this;
    }
}

void RequireUnitPort::disconnect()
{
    if (connection != NULL) {
        connection->connection = NULL;
        connection = NULL;
    }
}

AbstractAction* RequireUnitPort::prepareForExecution(AbstractExecutionEngine* engine)
{
    if (connection == NULL)
        return NULL;

    return connection->prepareForExecution(engine);
}
