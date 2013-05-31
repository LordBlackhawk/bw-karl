#pragma once

#include <BWAPI.h>
#include <BWTA.h>

class MicroMovement
{
    public:
        MicroMovement(BWAPI::Unit* u);
        virtual ~MicroMovement();

        // tick methods:
        bool onTick();
        virtual void onMacroTick();

        // query methods:
        bool isIdle();
        BWTA::Region* currentRegion();

        // commands:
        void moveTo(const BWAPI::Position& target);
        void scoutRegion(BWTA::Region* region);

    private:
        BWAPI::Unit* unit;
        int nextCommandFrame;
        int nextMacroFrame;

        enum InternalState { sIdle, sMoving, sCommandMove, sScoutRegion };
        InternalState state;
        InternalState finishState;

        BWAPI::Position targetPos;
        BWTA::Region* targetRegion;
};
