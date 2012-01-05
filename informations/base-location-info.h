#pragma once

#include <BWAPI.h>
#include <BWTA.h>

class BaseLocationInfo
{
	friend class InformationKeeper;
	
	public:
		BWTA::BaseLocation* get() const
		{
			return location;
		}
		
		BWAPI::TilePosition getTilePosition() const
		{
			return tilepos;
		}
		
		BWAPI::Position getPosition() const
		{
			return pos;
		}
		
		bool isStartLocation() const
		{
			return startlocation;
		}
		
		int lastSeen() const
		{
			return lastseen;
		}
		
		bool isVisible() const
		{
			return (lastseen == InformationKeeper::instance().currentFrame());
		}
		
		UnitInfoPtr currentBase() const
		{
			return currentbase;
		}
		
		PlayerInfoPtr currentUser() const
		{
			return currentuser;
		}
		
		RegionInfoPtr getRegion() const
		{
			return region;
		}
	
	protected:
		void readEveryTurn();
	
	protected:
		BWTA::BaseLocation* location;
		
		BWAPI::TilePosition tilepos;
		BWAPI::Position pos;
		bool startlocation;
		int lastseen;
		UnitInfoPtr currentbase;
		PlayerInfoPtr currentuser;
		RegionInfoPtr region;
		
		BaseLocationInfo(BWTA::BaseLocation* loc) : location(loc), lastseen(-1)
		{
			tilepos       = location->getTilePosition();
			pos           = location->getPosition();
			startlocation = location->isStartLocation();
			region        = InformationKeeper::instance().getInfo(location->getRegion());
		}
};
