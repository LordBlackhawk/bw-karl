#pragma once

#include "utils/singleton.h"

#include <BWAPI.h>
#include <BWTA.h>

#include <boost/shared_ptr.hpp>

#include <map>

class PlayerInfo;
class UnitInfo;
class BaseLocationInfo;
class RegionInfo;
class ChokepointInfo;

typedef boost::shared_ptr<PlayerInfo> PlayerInfoPtr;
typedef boost::shared_ptr<UnitInfo> UnitInfoPtr;
typedef boost::shared_ptr<BaseLocationInfo> BaseLocationInfoPtr;
typedef boost::shared_ptr<RegionInfo> RegionInfoPtr;
typedef boost::shared_ptr<ChokepointInfo> ChokepointInfoPtr;

class InformationKeeper
{
	friend class UnitInfo;
	
	public:
		static InformationKeeper& instance()
		{
			return Singleton<InformationKeeper>::instance();
		}
		
		void clear();
		void prepareMap();
		void pretick();
		void tick();
		
		int currentFrame() const
		{
			return currentframe;
		}
		
		int latencyFrames() const
		{
			return latencyframes;
		}
		
		BWAPI::Player* self() const
		{
			return _self;
		}
		
		BWAPI::Player* neutral() const
		{
			return _neutral;
		}
		
		PlayerInfoPtr       getInfo(BWAPI::Player* player);
		UnitInfoPtr         getInfo(BWAPI::Unit* unit);
		BaseLocationInfoPtr getInfo(BWTA::BaseLocation* loc);
		RegionInfoPtr       getInfo(BWTA::Region* region);
		ChokepointInfoPtr   getInfo(BWTA::Chokepoint* point);
		
	protected:
		void baseFound(BWAPI::Unit* base);
		
	protected:
		int currentframe;
		int latencyframes;
		
		BWAPI::Player* _self;
		BWAPI::Player* _neutral;
		
		std::map<BWAPI::Player*, PlayerInfoPtr>				players;
		std::map<BWAPI::Unit*, UnitInfoPtr>					units;
		std::map<BWTA::BaseLocation*, BaseLocationInfoPtr>	baselocations;
		std::map<BWTA::Region*, RegionInfoPtr>				regions;
		std::map<BWTA::Chokepoint*, ChokepointInfoPtr>		chokepoints;
};

#include "player-info.h"
#include "unit-info.h"
#include "base-location-info.h"
#include "region-info.h"
#include "choke-point-info.h"