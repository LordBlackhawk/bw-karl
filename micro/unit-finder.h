#pragma once

#include "micro-task-manager.h"

#include "utils/debug.h"

#include <BWAPI.h>

class UnitFinder
{
	public:
		static UnitFinder& instance()
		{
			return Singleton<UnitFinder>::instance();
		}
		
		struct ListType
		{
			enum Type { inactive, own, global };
		};
		
		std::set<BWAPI::Unit*> getList(const ListType::Type& type) const
		{
			switch (type)
			{
				case ListType::inactive:
					return MicroTaskManager::instance().inactiveUnits();
				case ListType::own:
					return MicroTaskManager::instance().allUnits();
				case ListType::global:
					return BWAPI::Broodwar->getAllUnits();
				default:
					return std::set<BWAPI::Unit*>();
			}
		}

		template <class Predicate>
		BWAPI::Unit* findFirst(const ListType::Type& type, const Predicate& pred) const
		{
			std::set<BWAPI::Unit*> list = getList(type);
			for (auto it : list) {
				if (pred(it))
					return it;
			}
			return NULL;
		}

		template <class Predicate>
		BWAPI::Unit* findBest(const ListType::Type& type, const Predicate& pred) const
		{
			BWAPI::Unit* best = NULL;
			double bestvalue = 0.;
			std::set<BWAPI::Unit*> list = getList(type);
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
			auto pred = [ut] (BWAPI::Unit* unit)
					{
						return (unit->getType() == ut);
					};
			BWAPI::Unit* result = findFirst(ListType::inactive, pred);
			if (result == NULL)
				LOG << "UnitFinder::findIdle(" << ut.getName() << ") unable to find!";
			return result;
		}

		BWAPI::Unit* find(const BWAPI::UnitType& ut, const BWAPI::TilePosition& pos) const
		{
			auto pred = [ut, pos] (BWAPI::Unit* unit)
					{
						return (unit->getType() == ut) && (unit->getTilePosition() == pos);
					};
			return findFirst(ListType::inactive, pred);
		}

		BWAPI::Unit* find(const BWAPI::UnitType& ut, const BWAPI::Position& pos) const
		{
			auto pred = [ut, pos] (BWAPI::Unit* unit)
					{
						return (unit->getType() == ut) && (pos.getDistance(unit->getPosition()) < 32);
					};
			return findFirst(ListType::inactive, pred);
		}
		
		BWAPI::Unit* findGlobal(const BWAPI::UnitType& ut, const BWAPI::TilePosition& pos) const
		{
			auto pred = [ut, pos] (BWAPI::Unit* unit)
					{
						return (unit->getType() == ut) && (unit->getTilePosition() == pos);
					};
			return findFirst(ListType::global, pred);
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
			return findBest(ListType::own, pred);
		}

		BWAPI::Unit* findWorker(const BWAPI::TilePosition& pos) const
		{
			auto pred = [pos] (BWAPI::Unit* unit)
					{
						if (!unit->getType().isWorker())
							return -1.;
						
						MicroTask task = MicroTaskManager::instance().activeTask(unit);
						if (!task.empty() && !task.isGatherMinerals())
							return -1.;
						
						return 1e5 - pos.getDistance(unit->getTilePosition());
					};
			return findBest(ListType::own, pred);
		}
		
		BWAPI::Unit* findWorker(const BWAPI::Position& pos) const
		{
			auto pred = [pos] (BWAPI::Unit* unit)
					{
						if (!unit->getType().isWorker())
							return -1.;
						
						MicroTask task = MicroTaskManager::instance().activeTask(unit);
						if (!task.empty() && !task.isGatherMinerals())
							return -1.;
						
						return 1e5 - pos.getDistance(unit->getPosition());
					};
			return findBest(ListType::own, pred);
		}

};