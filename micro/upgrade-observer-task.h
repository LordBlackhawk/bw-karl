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

		void tick(BWAPI::Unit* u)
		{
			unit = u;
			if (!unit->exists()) {
				failed(unit);
			} else if (unit->isUpgrading()) {
				// WAIT ...
			} else {
				completed(unit);
			}
			// TODO: isUnderAttack?
		}

	protected:
		BWAPI::Unit*		unit;
};

MicroTask createUpgradeObserver()
{
	MicroTaskData data(new UpgradeObserverTask());
	return MicroTask(MicroTaskEnum::UpgradeObserver, data);
} 
