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
        void acceptVisitor(AbstractEventVisitor* visitor);
};

class UnitUpdateEvent : public AbstractEvent
{
    public:
        BWAPI::Unit*    unit;
        BWAPI::UnitType unitType;
        BWAPI::Position pos;

        UnitUpdateEvent(BWAPI::Unit* u, BWAPI::UnitType t, BWAPI::Position p);
        void acceptVisitor(AbstractEventVisitor* visitor);
};

class UnitCreateEvent : public UnitUpdateEvent
{
    public:
        BWAPI::Player*  owner;

        UnitCreateEvent(BWAPI::Unit* u, BWAPI::UnitType t, BWAPI::Position p, BWAPI::Player* o);
        void acceptVisitor(AbstractEventVisitor* visitor);
};

class BroodwarEvent : public AbstractEvent
{
    public:
        BWAPI::Event    event;

        BroodwarEvent(const BWAPI::Event& e);
        void acceptVisitor(AbstractEventVisitor* visitor);
};
