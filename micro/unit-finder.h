#pragma once

#include "micro-task-manager.h"
#include <BWAPI.h>

class UnitFinder
{
	public:
		static UnitFinder& instance()
		{
			return Singleton<UnitFinder>::instance();
		}

		template <class Predicate>
		BWAPI::Unit* findFirst(const Predicate& pred) const
		{
			std::set<BWAPI::Unit*> list = MicroTaskManager::instance().inactiveUnits();
			for (auto it : list) {
				if (pred(it))
					return it;
			}
			return NULL;
		}

		template <class Predicate>
		BWAPI::Unit* findBest(const Predicate& pred) const
		{
			BWAPI::Unit* best = NULL;
			double bestvalue = 0.;
			std::set<BWAPI::Unit*> list = MicroTaskManager::instance().allUnits();
			for (auto it : list)
			{
				double value = pred(it);
				if (value > bestvalue) {
					best = it;
					bestvalue = value;
				}
			}
			return best;
		}

		BWAPI::Unit* findIdle(const BWAPI::UnitType& ut) const
		{
			//std::clog << "searching for UnitType " << ut.getName() << "...\n";
			auto pred = [ut] (BWAPI::Unit* unit)
					{
						//std::clog << "\tfound " << unit->getType().getName() << ".\n";
						return (unit->getType() == ut);
					};
			return findFirst(pred);
		}

		BWAPI::Unit* find(const BWAPI::UnitType& ut, const BWAPI::TilePosition& pos) const
		{
			auto pred = [ut, pos] (BWAPI::Unit* unit)
					{
						return (unit->getType() == ut) && (unit->getTilePosition() == pos);
					};
			return findFirst(pred);
		}

		BWAPI::Unit* find(const BWAPI::UnitType& ut, const BWAPI::Position& pos) const
		{
			auto pred = [ut, pos] (BWAPI::Unit* unit)
					{
						return (unit->getType() == ut) && (pos.getDistance(unit->getPosition()) < 32);
					};
			return findFirst(pred);
		}

		BWAPI::Unit* findWorker(const BWAPI::Race& race, const BWAPI::TilePosition& pos) const
		{
			BWAPI::UnitType ut = race.getWorker();
			auto pred = [ut, pos] (BWAPI::Unit* unit)
					{
						if (unit->getType() != ut)
							return -1.;
						
						MicroTask task = MicroTaskManager::instance().activeTask(unit);
						if (!task.empty() && !task.isGatherMinerals())
							return -1.;
						
						return 1e5 - pos.getDistance(unit->getTilePosition());
					};
			return findBest(pred);
		}

		BWAPI::Unit* findWorker(const BWAPI::TilePosition& pos) const
		{
			auto pred = [pos] (BWAPI::Unit* unit)
					{
						if (unit->getType().isWorker())
							return 1e5 - pos.getDistance(unit->getTilePosition());
						else
							return -1.;
					};
			return findBest(pred);
		}
		
		BWAPI::Unit* findWorker(const BWAPI::Position& pos) const
		{
			auto pred = [pos] (BWAPI::Unit* unit)
					{
						if (unit->getType().isWorker())
							return 1e5 - pos.getDistance(unit->getPosition());
						else
							return -1.;
					};
			return findBest(pred);
		}

};