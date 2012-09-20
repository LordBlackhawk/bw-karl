#pragma once

#include <BWAPI.h>

namespace {

	void drawBuildingPosition(const BWAPI::UnitType& ut, const BWAPI::TilePosition& tp)
	{
		BWAPI::Broodwar->drawBoxMap(tp.x()*32, tp.y()*32, tp.x()*32+ut.tileWidth()*32, tp.y()*32+ut.tileHeight()*32, BWAPI::Colors::Green);
		BWAPI::Broodwar->drawTextMap(tp.x()*32, tp.y()*32, "%s", ut.getName().c_str());
	}
	
	BWAPI::Position getBuildingCenter(const BWAPI::TilePosition& pos, const BWAPI::UnitType& ut)
	{
		BWAPI::Position result = BWAPI::Position(pos);
		result += BWAPI::Position(ut.tileWidth() * 16, ut.tileHeight() * 16);
		return result;
	}
	
}