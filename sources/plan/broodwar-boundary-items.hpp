#pragma once

#include "broodwar-ports.hpp"

class OwnUnitBoundaryItem : public AbstractBoundaryItem
{
    public:
        OwnUnitBoundaryItem(BWAPI::Unit* u);

        void acceptVisitor(AbstractVisitor* visitor) override;
        void visitUnitCreateEvent(UnitCreateEvent* event) override;
        void visitOwnUnitUpdateEvent(OwnUnitUpdateEvent* event) override;

        inline BWAPI::UnitType getUnitType() const { return provideUnit.getUnitType(); }
        inline bool isConnected() const { return provideUnit.isConnected(); }

    protected:
        ProvideUnitPort provideUnit;
};

class MineralBoundaryItem : public AbstractBoundaryItem
{
    public:
        MineralBoundaryItem(BWAPI::Unit* u, BaseLocation* b = NULL);
        ~MineralBoundaryItem();

        void acceptVisitor(AbstractVisitor* visitor) override;
        void visitUnitCreateEvent(UnitCreateEvent* event) override;
        void visitMineralUpdateEvent(MineralUpdateEvent* event) override;

        inline const BWAPI::TilePosition& getTilePosition() const { return pos; }
        inline int mineralsLeft() const { return minerals; }

    protected:
        BaseLocation*       base;
        BWAPI::TilePosition pos;
        int                 minerals;
};
