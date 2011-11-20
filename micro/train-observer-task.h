#pragma once

#include "base-task.h"
#include <BWAPI.h>

class TrainObserverTask : public BaseTask
{
	public:
		TrainObserverTask()
		{ }

		void activate(BWAPI::Unit* u)
		{
			unit = u;
		}

		TaskStatus::Type tick()
		{
			if (!unit->exists()) {
				return failed(unit);
			} else if (unit->isBeingConstructed()) {
				// WAIT ...
			} else {
				return completed(unit);
			}
			// TODO: isUnderAttack?
			return TaskStatus::running;
		}

	protected:
		BWAPI::Unit*		unit;
};

MicroTask createTrainObserver()
{
	MicroTaskData data(new TrainObserverTask());
	return MicroTask(MicroTaskEnum::TrainObserver, data);
} 
