#pragma once

#include <BWTA.h>

class ChokepointInfo
{
	friend class InformationKeeper;
	
	public:
		BWTA::Chokepoint* get() const
		{
			return point;
		}
		
		bool isVisible() const
		{
			return lastseen == InformationKeeper::instance().currentFrame();
		}
		
		int lastSeen() const
		{
			return lastseen;
		}
		
		BWAPI::Position getPosition() const
		{
			return pos;
		}
		
		BWAPI::TilePosition getTilePosition() const
		{
			return tilepos;
		}
		
		std::pair<RegionInfo, RegionInfo> getSides() const
		{
			return sides;
		}
		
		double getWidth() const
		{
			return width;
		}
		
	protected:
		void readEveryTurn();
	
	protected:
		BWTA::Chokepoint* point;
		
		int lastseen;
		BWAPI::Position pos;
		BWAPI::TilePosition tilepos;
		std::pair<RegionInfo, RegionInfo> sides;
		double width;
		
		ChokepointInfo(BWTA::Chokepoint* p) : point(p), visible(false)
		{
			pos = point->getCenter();
			tilepos = TilePosition(pos);
			auto s = point->getSides();
			sides = std::make_pair(InformationKeeper::instance().getInfo(s.first), InformationKeeper::instance().getInfo(s.second));
			width = point->getWidth();
			
			// ToDo: Blocked by Minerals ???
		}
};
