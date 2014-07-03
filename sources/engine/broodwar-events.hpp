#pragma once

#include "utils/time.hpp"
#include "abstract-action.hpp"
#include <BWAPI.h>

class FrameEvent : public AbstractEvent
{
    public:
        Time    currentTime;
        int     currentMinerals;
        int     currentGas;

        FrameEvent(Time t, int m, int g);
        void acceptVisitor(AbstractEventVisitor* visitor) override;
};

class UnitUpdateEvent : public AbstractEvent
{
    public:
        BWAPI::Unit*    unit;

        UnitUpdateEvent(BWAPI::Unit* u);
        void acceptVisitor(AbstractEventVisitor* visitor) override;
};

class OwnUnitUpdateEvent : public UnitUpdateEvent
{
    public:
        BWAPI::UnitType unitType;
        BWAPI::Position pos;

        OwnUnitUpdateEvent(BWAPI::Unit* u, BWAPI::UnitType t, BWAPI::Position p);
        void acceptVisitor(AbstractEventVisitor* visitor) override;
};

class MineralUpdateEvent : public UnitUpdateEvent
{
    public:
        int minerals;

        MineralUpdateEvent(BWAPI::Unit* u, int m);
        void acceptVisitor(AbstractEventVisitor* visitor) override;
};

class UnitCreateEvent : public AbstractEvent
{
    public:
        BWAPI::Unit*        unit;
        BWAPI::UnitType     unitType;
        BWAPI::Player*      owner;
        BWAPI::TilePosition tilePos;
        BWAPI::Position     pos;

        UnitCreateEvent(BWAPI::Unit* u, BWAPI::UnitType t, const BWAPI::TilePosition& tp, const BWAPI::Position& p, BWAPI::Player* o);
        void acceptVisitor(AbstractEventVisitor* visitor) override;
};

class BroodwarEvent : public AbstractEvent
{
    public:
        BWAPI::Event    event;

        BroodwarEvent(const BWAPI::Event& e);
        void acceptVisitor(AbstractEventVisitor* visitor) override;
};
