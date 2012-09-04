#pragma once

#include <BWAPI.h>
#include <string>

struct DefaultCode
{
	static void onMatchBegin()
	{ }
	
	static void onMatchEnd()
	{ }

    static void onTick()
    { }
	
	static void onMatchEndMessage(bool /*winner*/)
	{ }
	
	static void onSendText(const std::string& /*text*/)
	{ }
	
	static void onReceiveText(BWAPI::Player* /*player*/, const std::string& /*text*/)
	{ }
	
	static void onPlayerLeft(BWAPI::Player* /*player*/)
	{ }
	
	static void onNukeDetect(const BWAPI::Position& /*pos*/)
	{ }
	
	static void onUnitCreate(BWAPI::Unit* /*unit*/)
	{ }
	
	static void onUnitDestroy(BWAPI::Unit* /*unit*/)
	{ }
	
	static void onUnitMorph(BWAPI::Unit* /*unit*/)
	{ }
	
	static void onUnitShow(BWAPI::Unit* /*unit*/)
	{ }
	
	static void onUnitHide(BWAPI::Unit* /*unit*/)
	{ }
	
	static void onUnitRenegade(BWAPI::Unit* /*unit*/)
	{ }
};
