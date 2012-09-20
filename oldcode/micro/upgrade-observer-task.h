#pragma once

#include "base-task.h"
#include <BWAPI.h>

class UpgradeObserverTask : public BaseTask
{
	public:
		UpgradeObserverTask() : BaseTask(MicroTaskEnum::UpgradeObserver)
		{ }

		void activate(UnitInfoPtr u)
		{
			unit = u;
		}

		TaskStatus::Type tick()
		{
			if (!unit->get()->exists()) {
				return failed(unit);
			} else if (unit->get()->isUpgrading()) {
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

MicroTaskPtr createUpgradeObserver()
{
	return MicroTaskPtr(new UpgradeObserverTask());
} 
