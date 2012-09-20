#pragma once

#include "gather-minerals-task.h"
#include "gather-gas-task.h"
#include "unit-finder.h"
#include "settick.h"

#include "informations/informations.h"

#include "utils/debug.h"

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
			UnitInfoPtr unit = UnitFinder::instance().findWorker(best->getPosition());
			unit->pushTask(best);
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
		
		void updateBases()
		{
			int currentframe = InformationKeeper::instance().currentFrame();
			for (auto it : InformationKeeper::instance().self()->getBases())
				if (it->currentUserSince() == currentframe)
					mineralTasks.insert(GatherMineralsTaskPtr(new GatherMineralsTask(it)));
		}

		void useIdleWorker(UnitInfoPtr unit)
		{
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
				LOG2 << "Unable to use Worker! bestvalue is " << bestvalue;
				return;
			}
			unit->pushTask(best);
		}
		
		void useIdleExtractor(UnitInfoPtr unit)
		{
			GatherGasTaskPtr task = GatherGasTaskPtr(new GatherGasTask(unit));
			unit->pushTask(task);
			gasTasks.insert(task);
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
