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

ProvideMineralFieldPort::ProvideMineralFieldPort(MineralBoundaryItem* o)
    : BaseClass(o)
{
    estimatedTime = ACTIVE_TIME;
}

void ProvideMineralFieldPort::disconnect()
{
    if (isConnected()) {
        BasicPortImpl<ProvideMineralFieldPort, RequireMineralFieldPort, false>::disconnect();
        getOwner()->removePort(this);
        delete this;
    }
}

void ProvideMineralFieldPort::acceptVisitor(AbstractVisitor* visitor)
{
    visitor->visitProvideMineralFieldPort(this);
}

BWAPI::Unit* ProvideMineralFieldPort::getUnit() const
{
    return getOwner()->getUnit();
}

MineralBoundaryItem* ProvideMineralFieldPort::getOwner() const
{
    return static_cast<MineralBoundaryItem*>(owner);
}

RequireMineralFieldPort::RequireMineralFieldPort(AbstractItem* o, MineralBoundaryItem* m)
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

RequireSpacePort::RequireSpacePort(AbstractItem* o, Array2d<FieldInformations>* f, int w, int h, BWAPI::TilePosition p)
    : AbstractPort(o), fields(f), pos(BWAPI::TilePositions::Unknown), width(w), height(h)
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

void RequireSpacePort::disconnect()
{
    if (isConnected()) {
        for (int x=pos.x(); x<pos.x()+width; ++x)
            for (int y=pos.y(); y<pos.y()+height; ++y)
                (*fields)[x][y].blocker = NULL;
        pos = BWAPI::TilePositions::Unknown;
    }
}

void RequireSpacePort::connectTo(BWAPI::TilePosition tp)
{
    if (isConnected())
        disconnect();
    pos = tp;
    if (isConnected()) {
        for (int x=pos.x(); x<pos.x()+width; ++x)
            for (int y=pos.y(); y<pos.y()+height; ++y)
        {
            auto& field = (*fields)[x][y];
            if (field.blocker != NULL)
                field.blocker->disconnect();
            field.blocker = this;
        }
    }
}
