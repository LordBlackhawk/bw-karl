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

class BroodwarEvent : public AbstractEvent
{
    public:
        BWAPI::Event    event;

        BroodwarEvent(BWAPI::Event e);
        void acceptVisitor(AbstractEventVisitor* visitor);
};

class UnitEvent : public AbstractEvent
{
    public:
        BWAPI::Unit*    unit;
        BWAPI::UnitType unitType;
        BWAPI::Position pos;

        UnitEvent(BWAPI::Unit* u, BWAPI::UnitType t, BWAPI::Position p);
        void acceptVisitor(AbstractEventVisitor* visitor);
};
