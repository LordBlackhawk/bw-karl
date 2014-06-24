#pragma once

#include "plan-item.hpp"

#include <BWAPI.h>

class AbstractAction;
class ProvideUnitPort;
class RequireUnitPort;

class ProvideUnitPort : public AbstractPort
{
    public:
        ProvideUnitPort(BWAPI::Unit* u, bool od = false);
        ~ProvideUnitPort();
        void updateData(BWAPI::UnitType ut, BWAPI::Position p);
        void updateData(RequireUnitPort* port);

        bool isRequirePort() const override;
        bool isActiveConnection() const override;
        void acceptVisitor(AbstractVisitor* visitor) override;

        void connectTo(RequireUnitPort* port);
        void disconnect();
        AbstractAction* prepareForExecution(AbstractExecutionEngine* engine);

        inline bool isConnected() const { return connection != NULL; }
        inline BWAPI::Unit* getUnit() const { return unit; }
        inline BWAPI::UnitType getUnitType() const { return unitType; }
        inline BWAPI::Position getPosition() const { return pos; }
        inline bool isOnDemand() const { return onDemand; }

        inline void setUnit(BWAPI::Unit* u) { unit = u; }
        inline void setPreviousAction(AbstractAction* action) { previousAction = action; }

    protected:
        friend class RequireUnitPort;

        RequireUnitPort*    connection;
        BWAPI::Unit*        unit;
        BWAPI::UnitType     unitType;
        BWAPI::Position     pos;
        bool                onDemand;
        AbstractAction*     previousAction;
};

class RequireUnitPort : public AbstractPort
{
    public:
        RequireUnitPort(BWAPI::UnitType ut);
        ~RequireUnitPort();

        bool isRequirePort() const override;
        bool isActiveConnection() const override;
        void acceptVisitor(AbstractVisitor* visitor) override;

        void updateEstimates();
        void connectTo(ProvideUnitPort* port);
        void disconnect();
        void bridge(ProvideUnitPort* port);
        AbstractAction* prepareForExecution(AbstractExecutionEngine* engine);

        inline bool isConnected() const { return connection != NULL; }
        inline BWAPI::Unit* getUnit() const { return (connection != NULL) ? connection->unit : NULL; }
        inline BWAPI::UnitType getUnitType() const { return unitType; }
        inline BWAPI::Position getPosition() const { return (connection != NULL) ? connection->pos : BWAPI::Positions::Unknown; }

    protected:
        ProvideUnitPort*    connection;
        BWAPI::UnitType     unitType;
};

class ResourcePort : public AbstractPort
{
    public:
        ResourcePort(int m, int g);

        bool isRequirePort() const override;
        bool isActiveConnection() const override;
        void acceptVisitor(AbstractVisitor* visitor) override;

        inline int getMinerals() const { return minerals; }
        inline int getGas() const { return gas; }

    protected:
        int minerals;
        int gas;
};
