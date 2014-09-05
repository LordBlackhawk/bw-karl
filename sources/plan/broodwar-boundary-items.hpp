#pragma once

#include "broodwar-ports.hpp"

class AbstractSpaceUnitBoundaryItem : public AbstractBoundaryItem
{
    public:
        RequireSpacePort    requireSpace;

        AbstractSpaceUnitBoundaryItem(BWAPI::Unit* u, Array2d<FieldInformations>* f, BWAPI::UnitType ut = BWAPI::UnitTypes::Unknown);

        void visitCompleteUnitUpdateEvent(CompleteUnitUpdateEvent* event) override;
        virtual BWAPI::Position getPosition() const = 0;
        int getHealth() const;
        double getGroundDPS() const;

        inline BWAPI::TilePosition getTilePosition() const { return requireSpace.getTilePosition(); }
        inline BWAPI::UnitType getUnitType() const { return unitType; }
        inline bool isBuilding() const { return unitType.isBuilding(); }
        inline bool isFlying() const { return unitType.isFlyer(); }

    protected:
        BWAPI::UnitType     unitType;
};

class OwnUnitBoundaryItem : public AbstractSpaceUnitBoundaryItem
{
    public:
        ProvideUnitPort     provideUnit;
        SupplyPort          supply;

        OwnUnitBoundaryItem(BWAPI::Unit* u, BWAPI::UnitType ut, Array2d<FieldInformations>* f);

        void acceptVisitor(AbstractVisitor* visitor) override;
        void visitCompleteUnitUpdateEvent(CompleteUnitUpdateEvent* event) override;
        void visitSimpleUnitUpdateEvent(SimpleUnitUpdateEvent* event) override;

        inline BWAPI::Position getPosition() const { return provideUnit.getPosition(); }
        inline bool isConnected() const { return provideUnit.isConnected(); }
};

class ResourceBoundaryItem : public AbstractSpaceUnitBoundaryItem
{
    public:
        ResourceBoundaryItem(BWAPI::Unit* u, BWAPI::UnitType ut, BlackboardInformations* i, BaseLocation* b = NULL);
        ~ResourceBoundaryItem();

        void acceptVisitor(AbstractVisitor* visitor) override;
        void visitMineralUpdateEvent(MineralUpdateEvent* event) override;

        int numberOfWorkers() const;
        inline int mineralsLeft() const { return minerals; }
        inline Time getLastSeen() const { return lastSeen; }
        inline BWAPI::Position getPosition() const { return BWAPI::Position(getTilePosition()); }
        inline bool isVisible() const { return lastSeen > info->lastUpdateTime - 5; }

    protected:
        BlackboardInformations* info;
        Time                    lastSeen;
        BaseLocation*           base;
        int                     minerals;
};

class EnemyUnitBoundaryItem : public AbstractSpaceUnitBoundaryItem
{
    public:
        EnemyUnitBoundaryItem(BWAPI::Unit* u, BWAPI::UnitType ut, BlackboardInformations* i);

        void acceptVisitor(AbstractVisitor* visitor) override;
        void visitSimpleUnitUpdateEvent(SimpleUnitUpdateEvent* event) override;

        inline BWAPI::Position getPosition() const { return position; }
        inline Time getLastSeen() const { return lastSeen; }
        inline bool isVisible() const { return lastSeen > info->lastUpdateTime - 5; }

    protected:
        BlackboardInformations* info;
        Time                    lastSeen;
        BWAPI::Position         position;
};
