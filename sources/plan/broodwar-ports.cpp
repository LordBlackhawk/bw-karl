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
    return isActive();
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
    if (m != NULL) {
        auto provider = new ProvideMineralFieldPort(m);
        m->ports.push_back(provider);
        connectTo(provider);
    }
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
