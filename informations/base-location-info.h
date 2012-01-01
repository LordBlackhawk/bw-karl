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
		
		BWAPI::Unit* currentBase() const
		{
			return currentbase;
		}
		
		BWAPI::Player* currentUser() const
		{
			return currentuser;
		}
	
	protected:
		void readLastSeen();
	
	protected:
		BWTA::BaseLocation* location;
		
		BWAPI::TilePosition tilepos;
		BWAPI::Position pos;
		bool startlocation;
		int lastseen;
		BWAPI::Unit* currentbase;
		BWAPI::Player* currentuser;
		
		BaseLocationInfo(BWTA::BaseLocation* loc) : location(loc), lastseen(-1), currentbase(NULL), currentuser(NULL)
		{
			tilepos       = location->getTilePosition();
			pos           = location->getPosition();
			startlocation = location->isStartLocation();
		}
};
