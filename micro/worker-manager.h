#pragma once

#include "micro-task-manager.h"
#include "gather-minerals-task.h"
#include "gather-gas-task.h"
#include "unit-finder.h"

typedef boost::shared_ptr<GatherMineralsTask>	GatherMineralsTaskPtr;
typedef boost::shared_ptr<GatherGasTask>		GatherGasTaskPtr;

class WorkerManager
{
	public:
		static WorkerManager& instance()
		{
			return Singleton<WorkerManager>::instance();
		}

		void sendGasWorker()
		{
			GatherGasTaskPtr best;
			int bestvalue = 3;
			for (auto it : gasTasks) {
				int value = it->workercount();
				if (value < bestvalue) {
					bestvalue = value;
					best = it;
				}
			}
			if (bestvalue > 2)
				return;
			BWAPI::Unit* unit = UnitFinder::findWorker(bestvalue->geyser.getPosition());
			MicroTaskManager::instance().pushTask(unit, MicroTask(MicroTaskEnum::GatherGas, best));
		}

		void returnGasWorker()
		{
			GatherGasTaskPtr best;
			int bestvalue = -1;
			for (auto it : gasTasks) {
				int value = it->workercount();
				if (value > bestvalue) {
					bestvalue = value;
					best = it;
				}
			}
			if (bestvalue < 1)
				return;
			best->giveBackWorker();
		}

		void useIdleWorker(BWAPI::Unit* unit)
		{
			BWAPI::Position pos = unit->getPosition();
			GatherMineralsTaskPtr best;
			double bestvalue = std::numeric_limits<double>::max();
			for (auto it : mineralTasks) {
				BWAPI::Position target = it->getPosition();
				double value = pos.distance(target);
				if (value < bestvalue) {
					best = it;
					bestvalue = value;
				}
			}
			if (bestvalue > 1e5)
				return;
			MicroTaskManager::instance().pushTask(unit, MicroTask(MicroTaskEnum::GatherMinerals, best));
		}

		void clear()
		{
			mineralTasks.clear();
			gasTasks.clear();
		}

	protected:
		std::set<GatherMineralsTaskPtr>	mineralTasks;
		std::set<GatherGasTaskPtr>		gasTasks;
};
