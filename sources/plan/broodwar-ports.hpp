#pragma once

#include "plan-item.hpp"

#include <BWAPI.h>

class ProvideUnitPort;
class RequireUnitPort;

class ProvideUnitPort : public AbstractPort
{
    public:
        ProvideUnitPort(BWAPI::Unit* u);
        void updateData(BWAPI::UnitType ut, BWAPI::Position p);
        void updateData(RequireUnitPort* port);

        bool isRequirePort() const;
        void acceptVisitor(AbstractVisitor* visitor);

        void connectTo(RequireUnitPort* port);
        void disconnect();

        inline bool isConnected() const { return connection != NULL; }
        inline BWAPI::Unit* getUnit() const { return unit; }
        inline BWAPI::UnitType getUnitType() const { return unitType; }
        inline BWAPI::Position getPosition() const { return pos; }

    protected:
        friend class RequireUnitPort;

        RequireUnitPort*    connection;
        BWAPI::Unit*        unit;
        BWAPI::UnitType     unitType;
        BWAPI::Position     pos;
};

class RequireUnitPort : public AbstractPort
{
    public:
        RequireUnitPort(BWAPI::UnitType ut);

        bool isRequirePort() const;
        void acceptVisitor(AbstractVisitor* visitor);

        void updateEstimates();
        void connectTo(ProvideUnitPort* port);
        void disconnect();

        inline bool isConnected() const { return connection != NULL; }
        inline BWAPI::Unit* getUnit() const { return (connection != NULL) ? connection->unit : NULL; }
        inline BWAPI::UnitType getUnitType() const { return (connection != NULL) ? connection->unitType : BWAPI::UnitTypes::Unknown; }
        inline BWAPI::Position getPosition() const { return (connection != NULL) ? connection->pos : BWAPI::Positions::Unknown; }

    protected:
        ProvideUnitPort*    connection;
        BWAPI::UnitType     unitType;
};
