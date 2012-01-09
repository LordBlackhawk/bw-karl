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
		
		std::pair<RegionInfoPtr, RegionInfoPtr> getRegions() const
		{
			return regions;
		}
		
		RegionInfoPtr getOtherRegion(RegionInfoPtr r) const
		{
			return (regions.first == r) ? regions.second : regions.first;
		}
		
		BWAPI::Position getWaitingPosition(RegionInfoPtr r) const
		{
			return (regions.first == r) ? waitingpos.first : waitingpos.second;
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
		std::pair<RegionInfoPtr, RegionInfoPtr> regions;
		std::pair<BWAPI::Position, BWAPI::Position> waitingpos;
		double width;
		
		ChokepointInfo(BWTA::Chokepoint* p) : point(p)
		{
			pos = point->getCenter();
			tilepos = BWAPI::TilePosition(pos);
			auto s = point->getRegions();
			regions = std::make_pair(InformationKeeper::instance().getInfo(s.first), InformationKeeper::instance().getInfo(s.second));
			width = point->getWidth();
			
			auto pts = point->getSides();
			BWAPI::Position normal(-(pts.first.y() - pts.second.y()), pts.first.x() - pts.second.x());
			waitingpos.first  = BWAPI::Position(pos.x() + 2*normal.x(), pos.y() + 2*normal.y());
			waitingpos.second = BWAPI::Position(pos.x() - 2*normal.x(), pos.y() - 2*normal.y());
			
			if (InformationKeeper::instance().getRegion(waitingpos.first) != regions.first)
				std::swap(waitingpos.first, waitingpos.second);
			
			// ToDo: Blocked by Minerals ???
		}
};
