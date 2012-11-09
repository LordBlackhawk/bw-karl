#pragma once

#include "hud-text-output.hpp"
#include <BWAPI.h>
#include <BWTA.h>
#include <string>

struct DefaultCode
{
    // Once called at start of program.
    static void onReadParameter(int /*argc*/, const char** /*argv[]*/, int& /*cur*/)
    { }

    // Once called before the match begins.
    static void onMatchBegin()
    { }

    // Once called after the match has ended. Use only for finalization. Use onMatchEndMessage for post match analysis.
    static void onMatchEnd()
    { }

    // Once called per Frame.
    static void onTick()
    { }

    // Called by idle-unit-container to decide, whether the unit is added to the idle-unit-container. Return true, if this is not the case.
    static bool onAssignUnit(BWAPI::Unit* /*unit*/)
    {
        return false;
    }

    // Called after onMatchEnd to check, whether there are still objects alive. Class ObjectCounter supplies code.
    static void onCheckMemoryLeaks()
    { }

    // Called by hud-code, if plan should be drawn.
    static void onDrawPlan(HUDTextOutput& /*hud*/)
    { }
    
    // Called after base is mined out, strategie should overwrite.
    static void onBaseMinedOut(BWTA::BaseLocation* /*base*/)
    { }

    // BWAPI Message.
    static void onMatchEndMessage(bool /*winner*/)
    { }

    // BWAPI Message.
    static void onSendText(const std::string& /*text*/)
    { }

    // BWAPI Message.
    static void onReceiveText(BWAPI::Player* /*player*/, const std::string& /*text*/)
    { }

    // BWAPI Message.
    static void onPlayerLeft(BWAPI::Player* /*player*/)
    { }

    // BWAPI Message.
    static void onNukeDetect(const BWAPI::Position& /*pos*/)
    { }

    // BWAPI Message.
    static void onUnitCreate(BWAPI::Unit* /*unit*/)
    { }

    // BWAPI Message.
    static void onUnitDestroy(BWAPI::Unit* /*unit*/)
    { }

    // BWAPI Message.
    static void onUnitMorph(BWAPI::Unit* /*unit*/)
    { }

    // BWAPI Message.
    static void onUnitShow(BWAPI::Unit* /*unit*/)
    { }

    // BWAPI Message.
    static void onUnitHide(BWAPI::Unit* /*unit*/)
    { }

    // BWAPI Message.
    static void onUnitRenegade(BWAPI::Unit* /*unit*/)
    { }
};
