#pragma once

#include "log.hpp"
#include "default-code.hpp"

template <class ... List>
struct StrategieList
{ };

template <>
struct StrategieList<> : public DefaultCode
{
    enum { id = 0 };

    static void chooseStrategie()
    {
        WARNING << "No strategie is applyable!";
    }

    static void onMatchBeginInternal()
    { }
};

extern int idStrategie;

template <class First, class ... List>
struct StrategieList<First, List...>
{
    typedef StrategieList<List...> Next;

    enum { id = Next::id + 1 };

    static void onMatchBegin()
    {
        idStrategie = -1;
        chooseStrategie();
        onMatchBeginInternal();
    }

    static void chooseStrategie()
    {
        if (First::isApplyable()) {
            idStrategie = id;
        } else {
            Next::chooseStrategie();
        }
    }

    static void onMatchBeginInternal()
    {
        if (idStrategie == id)
            First::onMatchBegin();
        else
            Next::onMatchBeginInternal();
    }

    static void onProgramStart(const char* programname)
    {
        First::onProgramStart(programname);
        Next::onProgramStart(programname);
    }

    static void onReadParameter(int argc, const char* argv[], int& cur)
    {
        if (cur >= argc) return;
        First::onReadParameter(argc, argv, cur);
        Next::onReadParameter(argc, argv, cur);
    }

    static void onMatchEnd()
    {
        if (idStrategie == id)
            First::onMatchEnd();
        else
            Next::onMatchEnd();
    }

    static void onTick()
    {
        if (idStrategie == id)
            First::onTick();
        else
            Next::onTick();
    }

    static bool onAssignUnit(BWAPI::Unit* unit)
    {
        if (idStrategie == id)
            return First::onAssignUnit(unit);
        else
            return Next::onAssignUnit(unit);
    }

    static void onMatchEndMessage(bool winner)
    {
        if (idStrategie == id)
            First::onMatchEndMessage(winner);
        else
            Next::onMatchEndMessage(winner);
    }

    static void onSendText(const std::string& text)
    {
        if (idStrategie == id)
            First::onSendText(text);
        else
            Next::onSendText(text);
    }

    static void onReceiveText(BWAPI::Player* player, const std::string& text)
    {
        if (idStrategie == id)
            First::onReceiveText(player, text);
        else
            Next::onReceiveText(player, text);
    }

    static void onPlayerLeft(BWAPI::Player* player)
    {
        if (idStrategie == id)
            First::onPlayerLeft(player);
        else
            Next::onPlayerLeft(player);
    }

    static void onNukeDetect(const BWAPI::Position& pos)
    {
        if (idStrategie == id)
            First::onNukeDetect(pos);
        else
            Next::onNukeDetect(pos);
    }

    static void onUnitCreate(BWAPI::Unit* unit)
    {
        if (idStrategie == id)
            First::onUnitCreate(unit);
        else
            Next::onUnitCreate(unit);
    }

    static void onUnitDestroy(BWAPI::Unit* unit)
    {
        if (idStrategie == id)
            First::onUnitDestroy(unit);
        else
            Next::onUnitDestroy(unit);
    }

    static void onUnitMorph(BWAPI::Unit* unit)
    {
        if (idStrategie == id)
            First::onUnitMorph(unit);
        else
            Next::onUnitMorph(unit);
    }

    static void onUnitShow(BWAPI::Unit* unit)
    {
        if (idStrategie == id)
            First::onUnitShow(unit);
        else
            Next::onUnitShow(unit);
    }

    static void onUnitHide(BWAPI::Unit* unit)
    {
        if (idStrategie == id)
            First::onUnitHide(unit);
        else
            Next::onUnitHide(unit);
    }

    static void onUnitRenegade(BWAPI::Unit* unit)
    {
        if (idStrategie == id)
            First::onUnitRenegade(unit);
        else
            Next::onUnitRenegade(unit);
    }

    static void onCheckMemoryLeaks()
    {
        if (idStrategie == id)
            First::onCheckMemoryLeaks();
        else
            Next::onCheckMemoryLeaks();
    }

    static void onDrawPlan(HUDTextOutput& hud)
    {
        if (idStrategie == id)
            First::onDrawPlan(hud);
        else
            Next::onDrawPlan(hud);
    }

    static void onBaseMinedOut(BWTA::BaseLocation* base)
    {
        if (idStrategie == id)
            First::onBaseMinedOut(base);
        else
            Next::onBaseMinedOut(base);
    }
};
