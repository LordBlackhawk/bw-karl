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
		
		PlayerInfoPtr self() const
		{
			return _self;
		}
		
		PlayerInfoPtr neutral() const
		{
			return _neutral;
		}
		
		PlayerInfoPtr       getInfo(BWAPI::Player* player);
		UnitInfoPtr         getInfo(BWAPI::Unit* unit);
		BaseLocationInfoPtr getInfo(BWTA::BaseLocation* loc);
		RegionInfoPtr       getInfo(BWTA::Region* region);
		ChokepointInfoPtr   getInfo(BWTA::Chokepoint* point);
		
		BaseLocationInfoPtr getNearestFreeBase(const BWAPI::TilePosition& tilepos);
		
		/* bestway.h */
		RegionInfoPtr getRegion(const BWAPI::Position& pos);
		RegionInfoPtr getRegion(const BWAPI::TilePosition& tilepos);
		BaseLocationInfoPtr getNearestBaseLocation(const BWAPI::Position& pos);
		ChokepointInfoPtr getNearestChokepoint(const BWAPI::Position& pos);
		
		template <class FUNCTIONAL>
		double getBestWay(const FUNCTIONAL& f, const BWAPI::TilePosition& pos, const BWAPI::TilePosition& target, ChokepointInfoPtr& next);
		
		double getShortestWay(const BWAPI::TilePosition& pos, const BWAPI::TilePosition& target, ChokepointInfoPtr& next);
		
	protected:
		void baseFound(UnitInfoPtr base);
		void baseDestroyed(UnitInfoPtr base);
		
	protected:
		int currentframe;
		int latencyframes;
		
		PlayerInfoPtr _self;
		PlayerInfoPtr _neutral;
		
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