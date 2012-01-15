#pragma once

#include "informations/informations.h"
#include "utils/debug.h"
#include "micro-task.h"
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
			enum Type { idle, all };
		};
		
		const std::set<UnitInfoPtr>& getList(const ListType::Type& type) const
		{
			switch (type)
			{
				case ListType::idle:
					return InformationKeeper::instance().self()->idleUnits();
				case ListType::all:
					return InformationKeeper::instance().self()->allUnits();
				default:
					static std::set<UnitInfoPtr> emptyset;
					return emptyset;
			}
		}

		template <class Predicate>
		UnitInfoPtr findFirst(const ListType::Type& type, const Predicate& pred) const
		{
			for (auto it : getList(type)) {
				if (pred(it))
					return it;
			}
			return UnitInfoPtr();
		}

		template <class Predicate>
		UnitInfoPtr findBest(const ListType::Type& type, const Predicate& pred) const
		{
			UnitInfoPtr best = UnitInfoPtr();
			double bestvalue = 0.;
			for (auto it : getList(type))
			{
				double value = pred(it);
				if (value > bestvalue) {
					best = it;
					bestvalue = value;
				}
			}
			return best;
		}

		UnitInfoPtr findIdle(const BWAPI::UnitType& ut) const
		{
			auto pred = [ut] (UnitInfoPtr unit)
					{
						return (unit->getType() == ut);
					};
			UnitInfoPtr result = findFirst(ListType::idle, pred);
			if (result == NULL)
				LOG << "UnitFinder::findIdle(" << ut.getName() << ") unable to find!";
			return result;
		}

		UnitInfoPtr find(const BWAPI::UnitType& ut, const BWAPI::TilePosition& pos) const
		{
			auto pred = [ut, pos] (UnitInfoPtr unit)
					{
						return (unit->getType() == ut) && (unit->getTilePosition() == pos);
					};
			return findFirst(ListType::idle, pred);
		}

		UnitInfoPtr find(const BWAPI::UnitType& ut, const BWAPI::Position& pos) const
		{
			auto pred = [ut, pos] (UnitInfoPtr unit)
					{
						return (unit->getType() == ut) && (pos.getDistance(unit->getPosition()) < 32);
					};
			return findFirst(ListType::idle, pred);
		}
		
		/*
		UnitInfoPtr findGlobal(const BWAPI::UnitType& ut, const BWAPI::TilePosition& pos) const
		{
			auto pred = [ut, pos] (UnitInfoPtr unit)
					{
						return (unit->getType() == ut) && (unit->getTilePosition() == pos);
					};
			return findFirst(ListType::global, pred);
		}
		*/

		UnitInfoPtr findWorker(const BWAPI::Race& race, const BWAPI::TilePosition& pos) const
		{
			BWAPI::UnitType ut = race.getWorker();
			auto pred = [ut, pos] (UnitInfoPtr unit)
					{
						if (unit->getType() != ut)
							return -1.;
						
						MicroTaskPtr task = unit->currentTask();
						if (!task->empty() && !task->isGatherMinerals())
							return -1.;
						
						return 1e5 - pos.getDistance(unit->getTilePosition());
					};
			return findBest(ListType::all, pred);
		}

		UnitInfoPtr findWorker(const BWAPI::TilePosition& pos) const
		{
			auto pred = [pos] (UnitInfoPtr unit)
					{
						if (!unit->getType().isWorker())
							return -1.;
						
						MicroTaskPtr task = unit->currentTask();
						if (!task->empty() && !task->isGatherMinerals())
							return -1.;
						
						return 1e5 - pos.getDistance(unit->getTilePosition());
					};
			return findBest(ListType::all, pred);
		}
		
		UnitInfoPtr findWorker(const BWAPI::Position& pos) const
		{
			auto pred = [pos] (UnitInfoPtr unit)
					{
						if (!unit->getType().isWorker())
							return -1.;
						
						MicroTaskPtr task = unit->currentTask();
						if (!task->empty() && !task->isGatherMinerals())
							return -1.;
						
						return 1e5 - pos.getDistance(unit->getPosition());
					};
			return findBest(ListType::all, pred);
		}

};