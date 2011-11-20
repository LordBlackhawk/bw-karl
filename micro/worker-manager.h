#pragma once

#include "micro-task-manager.h"
#include "gather-minerals-task.h"
#include "gather-gas-task.h"
#include "unit-finder.h"
#include "settick.h"

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
				if (it->isWorking()) {
					int value = it->workercount();
					if (value < bestvalue) {
						bestvalue = value;
						best = it;
					}
				}
			}
			if (bestvalue > 2)
				return;
			BWAPI::Unit* unit = UnitFinder::instance().findWorker(best->getPosition());
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
			if (mineralTasks.size() < 1)
				mineralTasks.insert(GatherMineralsTaskPtr(new GatherMineralsTask(BWTA::getStartLocation(BWAPI::Broodwar->self()))));

			BWAPI::Position pos = unit->getPosition();
			GatherMineralsTaskPtr best;
			double bestvalue = std::numeric_limits<double>::max();
			for (auto it : mineralTasks) {
				BWAPI::Position target = it->getPosition(); // TODO
				double value = pos.getDistance(target);
				if (value < bestvalue) {
					best = it;
					bestvalue = value;
				}
			}
			if (bestvalue > 1e5) {
				std::clog << "Unable to use Worker! bestvalue is " << bestvalue << "\n";
				return;
			}
			MicroTaskManager::instance().pushTask(unit, MicroTask(MicroTaskEnum::GatherMinerals, best));
		}
		
		void useIdleExtractor(BWAPI::Unit* unit)
		{
			GatherGasTaskPtr data = GatherGasTaskPtr(new GatherGasTask(unit));
			MicroTaskManager::instance().pushTask(unit, MicroTask(MicroTaskEnum::GatherGas, data));
			gasTasks.insert(data);
		}

		void clear()
		{
			mineralTasks.clear();
			gasTasks.clear();
		}
		
		void tick()
		{
			settickptr(mineralTasks);
			settickptr(gasTasks);
		}

	protected:
		std::set<GatherMineralsTaskPtr>	mineralTasks;
		std::set<GatherGasTaskPtr>		gasTasks;
};
