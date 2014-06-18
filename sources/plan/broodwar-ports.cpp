#include "broodwar-ports.hpp"
#include "abstract-visitor.hpp"

ProvideUnitPort::ProvideUnitPort(BWAPI::Unit* u)
    : connection(NULL), unit(u), unitType(BWAPI::UnitTypes::Unknown), pos(BWAPI::Positions::Unknown)
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
