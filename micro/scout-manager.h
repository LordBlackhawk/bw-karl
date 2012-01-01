#pragma once

#include "micro-task-manager.h"
#include <BWAPI.h>
#include <BWTA.h>
#include <map>

class ScoutManager
{
	public:
		static ScoutManager& instance()
		{
			return Singleton<ScoutManager>::instance();
		}
		
		void clear()
		{
			units.clear();
			informations.clear();
		}
		
		void prepareMap()
		{
			for (auto it : BWTA::getBaseLocations())
				informations[it] = ScoutInformation(it->getPosition(), it->isStartLocation());
		}
		
		void useScout(BWAPI::Unit* unit)
		{
			BWTA::BaseLocation* target = getNextLocation(unit);
			if (target == NULL) {
				LOG << "ScoutManager: Find base location failed.";
				return;
			}
			MicroTask task = createScout(target);
			units[unit] = task;
			MicroTaskManager::instance().pushTask(unit, task);
		}
		
		int scoutCount() const
		{
			return units.size();
		}
		
		void tick()
		{
			std::set<BWAPI::Unit*> newjobs;
			auto it = units.begin();
			while (it != units.end())
			{
				switch(it->second.tick())
				{
					case TaskStatus::completed:
						processInformation(it->first);
						newjobs.insert(it->first);
					case TaskStatus::failed:
						units.erase(it);
						break;
						
					case TaskStatus::running:
						++it;
						break;
				}
			}
			
			for (auto it : newjobs)
				useScout(it);
		}

	protected:
		struct ScoutInformation
		{
			enum { factorAirGround     = 1000 };
			enum { factorFailed        =  500 };
			enum { factorDistance      =    1 };
			enum { factorStartLocation = 1000 };
		
			BWAPI::Position basepos;
			bool startlocation;
			
			int lastTimeSeen;
			int lastScoutSent;
			int failedGroundScouting;
			int failedAirScouting;
			
			ScoutInformation()
			{ }
			
			ScoutInformation(const BWAPI::Position& p, bool sl)
				: basepos(p), startlocation(sl), lastTimeSeen(0), lastScoutSent(0), failedGroundScouting(0), failedAirScouting(0)
			{ }
			
			double getValue(const BWAPI::Position& pos, bool flyer) const
			{
				return std::max(lastTimeSeen, lastScoutSent) 
						+ factorAirGround * ((flyer) ? -1 : +1) * (failedGroundScouting - failedAirScouting)
						+ factorFailed    * (failedGroundScouting + failedAirScouting)
						+ factorDistance  * basepos.getDistance(pos)
						- (startlocation) ? factorStartLocation : 0;
			}
		};
		
		std::map<BWAPI::Unit*, MicroTask>					units;
		std::map<BWTA::BaseLocation*, ScoutInformation>		informations;
		
		BWTA::BaseLocation* getNextLocation(BWAPI::Unit* unit)
		{
			bool flyer = unit->getType().isFlyer();
			BWAPI::Position pos = unit->getPosition();
			BWTA::BaseLocation* best = NULL;
			int bestvalue = std::numeric_limits<int>::max();
			for (auto it : informations) {
				int value = it.second.getValue(pos, flyer);
				if (value < bestvalue) {
					bestvalue = value;
					best = it.first;
				}
			}
			if (best == NULL)
				return NULL;
			ScoutInformation& info = informations[best];
			info.lastScoutSent     = InformationKeeper::instance().currentFrame();
			if (flyer)
				++info.failedAirScouting;
			else
				++info.failedGroundScouting;
			return best;
		}
		
		void processInformation(BWAPI::Unit* unit)
		{
			BWTA::BaseLocation* base = BWTA::getNearestBaseLocation(unit->getTilePosition());
			double dis = base->getPosition().getDistance(unit->getPosition());
			if (dis > 96.)
				return;
			ScoutInformation& info   = informations[base];
			info.lastTimeSeen        = InformationKeeper::instance().currentFrame();
			info.failedGroundScouting = 0;
			info.failedAirScouting   = 0;
		}
};
