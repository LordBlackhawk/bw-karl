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

class SimpleUnitUpdateEvent : public UnitUpdateEvent
{
    public:
        BWAPI::Position pos;

        SimpleUnitUpdateEvent(BWAPI::Unit* u, BWAPI::Position p);
        void acceptVisitor(AbstractEventVisitor* visitor) override;
};

class CompleteUnitUpdateEvent : public SimpleUnitUpdateEvent
{
    public:
        BWAPI::UnitType     unitType;
        BWAPI::Player*      owner;
        BWAPI::TilePosition tilePos;

        CompleteUnitUpdateEvent(BWAPI::Unit* u, BWAPI::UnitType t, const BWAPI::TilePosition& tp, const BWAPI::Position& p, BWAPI::Player* o);
        void acceptVisitor(AbstractEventVisitor* visitor) override;
};

class ResourceUpdateEvent : public UnitUpdateEvent
{
    public:
        int resources;

        ResourceUpdateEvent(BWAPI::Unit* u, int res);
        void acceptVisitor(AbstractEventVisitor* visitor) override;
};

class BroodwarEvent : public AbstractEvent
{
    public:
        BWAPI::Event    event;

        BroodwarEvent(const BWAPI::Event& e);
        void acceptVisitor(AbstractEventVisitor* visitor) override;
};

class FieldSeenEvent : public AbstractEvent
{
    public:
        BWAPI::TilePosition tilePos;
        bool creep;

        FieldSeenEvent(const BWAPI::TilePosition& tp, bool c);
        void acceptVisitor(AbstractEventVisitor* visitor) override;
};

class AbstractActionEvent : public AbstractEvent
{
    public:
        AbstractAction* sender;

        AbstractActionEvent(AbstractAction* a);
};

class ResourcesConsumedEvent : public AbstractActionEvent
{
    public:
        ResourcesConsumedEvent(AbstractAction* a);
        void acceptVisitor(AbstractEventVisitor* visitor) override;
};
