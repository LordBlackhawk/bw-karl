#pragma once

#include "base-task.h"
#include <BWAPI.h>

class TrainObserverTask : public BaseTask
{
	public:
		TrainObserverTask() : BaseTask(MicroTaskEnum::TrainObserver)
		{ }

		void activate(UnitInfoPtr u)
		{
			unit = u;
		}

		TaskStatus::Type tick()
		{
			if (!unit->get()->exists()) {
				return failed(unit);
			} else if (unit->get()->isBeingConstructed()) {
				// WAIT ...
			} else {
				return completed(unit);
			}
			// TODO: isUnderAttack?
			return TaskStatus::running;
		}

	protected:
		UnitInfoPtr		unit;
};

MicroTaskPtr createTrainObserver()
{
	return MicroTaskPtr(new TrainObserverTask());
} 
