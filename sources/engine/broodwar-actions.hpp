#pragma once

#include "abstract-action.hpp"
#include <BWAPI.h>

class UnitAction : public AbstractAction
{
    public:
        UnitAction(BWAPI::Unit* u, AbstractAction* pre = NULL);
        void drawInformations(const char* name);

    protected:
        BWAPI::Unit*    unit;
};

class CollectMineralsAction : public UnitAction
{
    public:
        CollectMineralsAction(BWAPI::Unit* w, BWAPI::Unit* m, AbstractAction* pre = NULL);
        Status onTick(AbstractExecutionEngine* engine) override;

    protected:
        BWAPI::Unit*    mineral;
};

class ZergBuildAction : public UnitAction
{
    public:
        ZergBuildAction(BWAPI::Unit* w, BWAPI::UnitType ut, BWAPI::TilePosition p, AbstractAction* pre = NULL);
        void onBegin(AbstractExecutionEngine* engine) override;
        Status onTick(AbstractExecutionEngine* engine) override;
        void onEnd(AbstractExecutionEngine* engine) override;

    protected:
        BWAPI::UnitType     unitType;
        BWAPI::TilePosition pos;
};

class MorphUnitAction : public UnitAction
{
    public:
        MorphUnitAction(BWAPI::Unit *u, BWAPI::UnitType to, AbstractAction *pre = NULL);
        void onBegin(AbstractExecutionEngine* engine) override;
        Status onTick(AbstractExecutionEngine* engine) override;
        void onEnd(AbstractExecutionEngine* engine) override;

    protected:
        BWAPI::UnitType     unitType;
};

class MoveToPositionAction : public UnitAction
{
    public:
        MoveToPositionAction(BWAPI::Unit* w, BWAPI::Position p, AbstractAction* pre = NULL);
        Status onTick(AbstractExecutionEngine* engine) override;

    protected:
        BWAPI::Position     pos;
};

class MineralTrigger : public AbstractAction
{
    public:
        MineralTrigger(int a, AbstractAction* pre = NULL);
        Status onTick(AbstractExecutionEngine* engine) override;

    protected:
        int amount;
};


class SendTextAction : public AbstractAction
{
    public:
        SendTextAction(std::string msg, bool toAlliesOnly=false, AbstractAction* pre = NULL);
        void onBegin(AbstractExecutionEngine* engine) override;
        Status onTick(AbstractExecutionEngine* engine) override;

    protected:
        std::string message;
        bool toAllies;
        int timeout;
};

class GiveUpAction : public SendTextAction
{
    public:
        GiveUpAction(AbstractAction* pre = NULL);
        Status onTick(AbstractExecutionEngine* engine) override;

    protected:
};