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

		void tick(BWAPI::Unit* u)
		{
			unit = u;
			if (!unit->exists()) {
				failed(unit);
			} else if (unit->isBeingConstructed()) {
				// WAIT ...
			} else {
				completed(unit);
			}
			// TODO: isUnderAttack?
		}

	protected:
		BWAPI::Unit*		unit;
};

MicroTask createTrainObserver()
{
	MicroTaskData data(new TrainObserverTask());
	return MicroTask(MicroTaskEnum::TrainObserver, data);
} 
