#pragma once

#include "broodwar-ports.hpp"

class AbstractSpaceUnitBoundaryItem : public AbstractBoundaryItem
{
    public:
        RequireSpacePort    requireSpace;
        
        AbstractSpaceUnitBoundaryItem(BWAPI::Unit* u, Array2d<FieldInformations>* f, BWAPI::UnitType ut = BWAPI::UnitTypes::Unknown);

        void visitCompleteUnitUpdateEvent(CompleteUnitUpdateEvent* event) override;

        inline BWAPI::TilePosition getTilePosition() const { return requireSpace.getTilePosition(); }
        inline BWAPI::UnitType getUnitType() const { return unitType; }

    protected:
        BWAPI::UnitType     unitType;
};

class OwnUnitBoundaryItem : public AbstractSpaceUnitBoundaryItem
{
    public:
        ProvideUnitPort     provideUnit;

        OwnUnitBoundaryItem(BWAPI::Unit* u, Array2d<FieldInformations>* f);

        void acceptVisitor(AbstractVisitor* visitor) override;
        void visitCompleteUnitUpdateEvent(CompleteUnitUpdateEvent* event) override;
        void visitSimpleUnitUpdateEvent(SimpleUnitUpdateEvent* event) override;

        inline BWAPI::Position getPosition() const { return provideUnit.getPosition(); }
        inline bool isConnected() const { return provideUnit.isConnected(); }
};

class ResourceBoundaryItem : public AbstractSpaceUnitBoundaryItem
{
    public:
        ResourceBoundaryItem(BWAPI::Unit* u, BWAPI::UnitType ut, Array2d<FieldInformations>* f, BaseLocation* b = NULL);
        ~ResourceBoundaryItem();

        void acceptVisitor(AbstractVisitor* visitor) override;
        void visitMineralUpdateEvent(MineralUpdateEvent* event) override;

        int numberOfWorkers() const;
        inline int mineralsLeft() const { return minerals; }

    protected:
        BaseLocation*       base;
        int                 minerals;
};

class EnemyUnitBoundaryItem : public AbstractSpaceUnitBoundaryItem
{
    public:
        EnemyUnitBoundaryItem(BWAPI::Unit* u, BWAPI::UnitType ut, Array2d<FieldInformations>* f);

        void acceptVisitor(AbstractVisitor* visitor) override;
        void visitSimpleUnitUpdateEvent(SimpleUnitUpdateEvent* event) override;

        inline BWAPI::Position getPosition() const { return position; }

    protected:
        BWAPI::Position     position;
};
