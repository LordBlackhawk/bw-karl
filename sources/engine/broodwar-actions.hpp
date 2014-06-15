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
        Status onTick(AbstractExecutionEngine* engine);

    protected:
        BWAPI::Unit*    mineral;
};

class ZergBuildAction : public UnitAction
{
    public:
        ZergBuildAction(BWAPI::Unit* w, BWAPI::UnitType ut, BWAPI::TilePosition p, AbstractAction* pre = NULL);
        void onBegin(AbstractExecutionEngine* engine);
        Status onTick(AbstractExecutionEngine* engine);
        void onEnd(AbstractExecutionEngine* engine);

    protected:
        BWAPI::UnitType     unitType;
        BWAPI::TilePosition pos;
};

class MineralTrigger : public AbstractAction
{
    public:
        MineralTrigger(int a, AbstractAction* pre = NULL);
        Status onTick(AbstractExecutionEngine* engine);

    protected:
        int amount;
};
