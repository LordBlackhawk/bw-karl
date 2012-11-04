#pragma once

#include "log.hpp"
#include "default-code.hpp"

template <class ... List>
struct CodeCaller
{ };

template <>
struct CodeCaller<> : public DefaultCode
{ };

template <class First, class ... List>
struct CodeCaller<First, List...>
{
    typedef CodeCaller<List...> Next;
    
    static void onReadParameter(int argc, const char* argv[], int& cur)
    {
        if (cur >= argc) return;
        First::onReadParameter(argc, argv, cur);
        Next::onReadParameter(argc, argv, cur);
    }
    
    static void readParameter(int argc, const char* argv[])
    {
        int cur = 1;
        while (cur < argc) {
            int backup = cur;
            onReadParameter(argc, argv, cur);
            if (backup == cur) {
                WARNING << "Unknown parameter '" << argv[cur] << "' is ignored.";
                ++cur;
            }
        }
    }
	
	static void onMatchBegin()
	{
		First::onMatchBegin();
		Next::onMatchBegin();
	}
	
	static void onMatchEnd()
	{
		// Note that onMatchEnd is called in the opposite ordering of onMatchBegin.
		Next::onMatchEnd();
		First::onMatchEnd();
	}

    static void onTick()
    {
        First::onTick();
        Next::onTick();
    }
	
	static bool onAssignUnit(BWAPI::Unit* unit)
	{
		if (First::onAssignUnit(unit))
			return true;
		return Next::onAssignUnit(unit);
	}
	
	static void onMatchEndMessage(bool winner)
	{
		First::onMatchEndMessage(winner);
		Next::onMatchEndMessage(winner);
	}
	
	static void onSendText(const std::string& text)
	{
		First::onSendText(text);
		Next::onSendText(text);
	}
	
	static void onReceiveText(BWAPI::Player* player, const std::string& text)
	{
		First::onReceiveText(player, text);
		Next::onReceiveText(player, text);
	}
	
	static void onPlayerLeft(BWAPI::Player* player)
	{
		First::onPlayerLeft(player);
		Next::onPlayerLeft(player);
	}
	
	static void onNukeDetect(const BWAPI::Position& pos)
	{
		First::onNukeDetect(pos);
		Next::onNukeDetect(pos);
	}
	
	static void onUnitCreate(BWAPI::Unit* unit)
	{
		First::onUnitCreate(unit);
		Next::onUnitCreate(unit);
	}
	
	static void onUnitDestroy(BWAPI::Unit* unit)
	{
		First::onUnitDestroy(unit);
		Next::onUnitDestroy(unit);
	}
	
	static void onUnitMorph(BWAPI::Unit* unit)
	{
		First::onUnitMorph(unit);
		Next::onUnitMorph(unit);
	}
	
	static void onUnitShow(BWAPI::Unit* unit)
	{
		First::onUnitShow(unit);
		Next::onUnitShow(unit);
	}
	
	static void onUnitHide(BWAPI::Unit* unit)
	{
		First::onUnitHide(unit);
		Next::onUnitHide(unit);
	}
	
	static void onUnitRenegade(BWAPI::Unit* unit)
	{
		First::onUnitRenegade(unit);
		Next::onUnitRenegade(unit);
	}
	
	static void onCheckMemoryLeaks()
	{
		First::onCheckMemoryLeaks();
		Next::onCheckMemoryLeaks();
	}
	
	static void onDrawPlan()
	{
		First::onDrawPlan();
		Next::onDrawPlan();
	}
};
