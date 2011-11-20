#pragma once

#include "base-task.h"
#include <BWAPI.h>

class UpgradeObserverTask : public BaseTask
{
	public:
		UpgradeObserverTask()
		{ }

		void activate(BWAPI::Unit* u)
		{
			unit = u;
		}

		TaskStatus::Type tick()
		{
			if (!unit->exists()) {
				return failed(unit);
			} else if (unit->isUpgrading()) {
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

MicroTask createUpgradeObserver()
{
	MicroTaskData data(new UpgradeObserverTask());
	return MicroTask(MicroTaskEnum::UpgradeObserver, data);
} 
