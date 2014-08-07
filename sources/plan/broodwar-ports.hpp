#pragma once

#include "basic-port-impl.hpp"

#include <BWAPI.h>

class AbstractAction;
class ProvideUnitPort;
class RequireUnitPort;
class ProvideMineralFieldPort;
class RequireMineralFieldPort;

class ProvideUnitPort final : public BasicPortImpl<ProvideUnitPort, RequireUnitPort, false, false>
{
    public:
        ProvideUnitPort(AbstractItem* o, BWAPI::Unit* u, bool od = false);

        void updateData(BWAPI::UnitType ut, BWAPI::Position p);
        void updateData(RequireUnitPort* port);

        void acceptVisitor(AbstractVisitor* visitor) override;

        AbstractAction* prepareForExecution(AbstractExecutionEngine* engine);

        inline BWAPI::Unit* getUnit() const { return unit; }
        inline BWAPI::UnitType getUnitType() const { return unitType; }
        inline BWAPI::Position getPosition() const { return pos; }
        inline bool isOnDemand() const { return onDemand; }

        inline void setUnit(BWAPI::Unit* u) { unit = u; }
        inline void setPreviousAction(AbstractAction* action) { previousAction = action; }

    protected:
        friend class RequireUnitPort;

        BWAPI::Unit*        unit;
        BWAPI::UnitType     unitType;
        BWAPI::Position     pos;
        bool                onDemand;
        AbstractAction*     previousAction;
};

class RequireUnitPort final : public BasicPortImpl<RequireUnitPort, ProvideUnitPort, true, false>
{
    public:
        RequireUnitPort(AbstractItem* o, BWAPI::UnitType ut);

        void acceptVisitor(AbstractVisitor* visitor) override;

        void bridge(ProvideUnitPort* port);
        AbstractAction* prepareForExecution(AbstractExecutionEngine* engine);

        inline BWAPI::Unit* getUnit() const { return (connection != NULL) ? connection->unit : NULL; }
        inline BWAPI::UnitType getUnitType() const { return unitType; }
        inline BWAPI::Position getPosition() const { return (connection != NULL) ? connection->pos : BWAPI::Positions::Unknown; }

    protected:
        BWAPI::UnitType     unitType;
};

class ResourcePort final : public AbstractPort
{
    public:
        ResourcePort(AbstractItem* o, int m, int g);

        bool isRequirePort() const override;
        bool isActiveConnection() const override;
        void acceptVisitor(AbstractVisitor* visitor) override;
        void disconnect() override;

        inline int getMinerals() const { return minerals; }
        inline int getGas() const { return gas; }

    protected:
        int minerals;
        int gas;
};

class ResourceBoundaryItem;

class ProvideMineralFieldPort final : public BasicPortImpl<ProvideMineralFieldPort, RequireMineralFieldPort, false, true>
{
    public:
        ProvideMineralFieldPort(ResourceBoundaryItem* o);
        void acceptVisitor(AbstractVisitor* visitor) override;

        BWAPI::Unit* getUnit() const;
        ResourceBoundaryItem* getOwner() const;
};

class RequireMineralFieldPort final : public BasicPortImpl<RequireMineralFieldPort, ProvideMineralFieldPort, true, false>
{
    public:
        RequireMineralFieldPort(AbstractItem* o, ResourceBoundaryItem* m);
        void acceptVisitor(AbstractVisitor* visitor) override;

        inline BWAPI::Unit* getUnit() const { return connection->getUnit(); }
};

struct FieldInformations;
template <class T> class Array2d;

class RequireSpacePort final : public AbstractPort
{
    public:
        RequireSpacePort(AbstractItem* o, Array2d<FieldInformations>* f, BWAPI::UnitType ut, BWAPI::TilePosition p = BWAPI::TilePositions::Unknown);
        ~RequireSpacePort();

        bool isRequirePort() const override;
        bool isActiveConnection() const override;
        void acceptVisitor(AbstractVisitor* visitor) override;

        void updateEstimates();
        void disconnect() override;
        void connectTo(BWAPI::TilePosition tp);
        void setUnitType(BWAPI::UnitType ut);

        inline const BWAPI::TilePosition& getTilePosition() const { return pos; }
        inline int getWidth() const { return unitType.tileWidth(); }
        inline int getHeight() const { return unitType.tileHeight(); }
        inline BWAPI::UnitType getUnitType() const { return unitType; }
        inline bool isConnected() const { return (pos != BWAPI::TilePositions::Unknown); }

    protected:
        Array2d<FieldInformations>* fields;
        BWAPI::TilePosition         pos;
        BWAPI::UnitType             unitType;
};
