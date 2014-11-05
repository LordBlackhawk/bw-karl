#pragma once

#include "broodwar-ports.hpp"
#include "utils/bw-helper.hpp"

class AbstractSpaceUnitBoundaryItem : public AbstractBoundaryItem
{
    public:
        RequireSpacePort    requireSpace;

        AbstractSpaceUnitBoundaryItem(BWAPI::Unit* u, BlackboardInformations* i, BWAPI::UnitType ut = BWAPI::UnitTypes::Unknown);

        void visitCompleteUnitUpdateEvent(CompleteUnitUpdateEvent* event) override;
        virtual BWAPI::Position getPosition() const = 0;
        int getMaxHealth() const;
        double getGroundDPS() const;

        void takeConnectionsFrom(AbstractBoundaryItem* other);

        inline BWAPI::TilePosition getTilePosition() const { return requireSpace.getTilePosition(); }
        inline BWAPI::UnitType getUnitType() const { return unitType; }
        inline bool isBuilding() const { return unitType.isBuilding(); }
        inline bool isFlying() const { return unitType.isFlyer(); }

    protected:
        BWAPI::UnitType         unitType;
        BlackboardInformations* info;
};

class OwnHatcheryBoundaryItem;

class OwnUnitBoundaryItem : public AbstractSpaceUnitBoundaryItem
{
    public:
        ProvideUnitPort     provideUnit;
        SupplyPort          supply;

        OwnUnitBoundaryItem(BWAPI::Unit* u, BWAPI::UnitType ut, BlackboardInformations* i);

        void acceptVisitor(AbstractVisitor* visitor) override;
        void visitCompleteUnitUpdateEvent(CompleteUnitUpdateEvent* event) override;
        void visitSimpleUnitUpdateEvent(SimpleUnitUpdateEvent* event) override;

        void takeConnectionsFrom(AbstractBoundaryItem* other);

        inline BWAPI::Position getPosition() const { return provideUnit.getPosition(); }
        inline bool isConnected() const { return provideUnit.isConnected(); }
        inline int getHealth() const { return health; }

    protected:
        int health;
};

class OwnHatcheryBoundaryItem : public OwnUnitBoundaryItem
{
    public:
        OwnHatcheryBoundaryItem(BWAPI::Unit* u, BWAPI::UnitType ut, BlackboardInformations* i);
        ~OwnHatcheryBoundaryItem();

        void acceptVisitor(AbstractVisitor* visitor) override;
        void visitSimpleUnitUpdateEvent(SimpleUnitUpdateEvent* event) override;

        Time lastPlanedLarva() const;
        ProvideUnitPort* createNewLarva();
        OwnUnitBoundaryItem* removeFirstPlanedLarva();

        inline const std::vector<OwnUnitBoundaryItem*> getLarvas() const { return larvas; }

    protected:
        std::vector<OwnUnitBoundaryItem*>   larvas;
};

class ResourceBoundaryItem : public AbstractSpaceUnitBoundaryItem
{
    public:
        ResourceBoundaryItem(BWAPI::Unit* u, BWAPI::UnitType ut, BlackboardInformations* i, BaseLocation* b = NULL);
        ~ResourceBoundaryItem();

        void acceptVisitor(AbstractVisitor* visitor) override;
        void visitResourceUpdateEvent(ResourceUpdateEvent* event) override;

        int numberOfWorkers() const;
        inline int mineralsLeft() const { return minerals; }
        inline int gasLeft() const { return gas; }
        inline Time getLastSeen() const { return lastSeen; }
        inline BWAPI::Position getPosition() const { return BWAPI::Position(getTilePosition()); }
        inline bool isVisible() const { return lastSeen > info->lastUpdateTime - 5; }

    protected:
        BlackboardInformations* info;
        Time                    lastSeen;
        BaseLocation*           base;
        int                     minerals;
        int                     gas;
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
        inline int getHealth() const { return health; }
        inline bool isGatheringMinerals() const { return currentAction == BWAction::GatheringMinerals; }
        inline bool isGatheringGas() const { return currentAction == BWAction::GatheringGas; }
        inline bool isBeingConstructed() const { return currentAction == BWAction::BeingConstructed; }

    protected:
        BlackboardInformations* info;
        Time                    lastSeen;
        BWAPI::Position         position;
        int                     health;
        BWAction::Type          currentAction;
};
