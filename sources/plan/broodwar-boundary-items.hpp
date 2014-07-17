#pragma once

#include "broodwar-ports.hpp"

class OwnUnitBoundaryItem : public AbstractBoundaryItem
{
    public:
        OwnUnitBoundaryItem(BWAPI::Unit* u, Array2d<FieldInformations>* f);

        void acceptVisitor(AbstractVisitor* visitor) override;
        void visitUnitCreateEvent(UnitCreateEvent* event) override;
        void visitOwnUnitUpdateEvent(OwnUnitUpdateEvent* event) override;

        inline BWAPI::UnitType getUnitType() const { return provideUnit.getUnitType(); }
        inline bool isConnected() const { return provideUnit.isConnected(); }

    protected:
        ProvideUnitPort     provideUnit;
        RequireSpacePort    requireSpace;
};

class ResourceBoundaryItem : public AbstractBoundaryItem
{
    public:
        ResourceBoundaryItem(BWAPI::Unit* u, BWAPI::UnitType ut, Array2d<FieldInformations>* f, BaseLocation* b = NULL);
        ~ResourceBoundaryItem();

        void acceptVisitor(AbstractVisitor* visitor) override;
        void visitUnitCreateEvent(UnitCreateEvent* event) override;
        void visitMineralUpdateEvent(MineralUpdateEvent* event) override;

        inline const BWAPI::TilePosition& getTilePosition() const { return requireSpace.getTilePosition(); }
        inline int mineralsLeft() const { return minerals; }

    protected:
        BWAPI::UnitType     unitType;
        RequireSpacePort    requireSpace;
        BaseLocation*       base;
        int                 minerals;
};
