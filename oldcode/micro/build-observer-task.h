#pragma once

#include "base-task.h"
#include <BWAPI.h>

class BuildObserverTask : public BaseTask
{
	public:
		BuildObserverTask() : BaseTask(MicroTaskEnum::BuildObserver)
		{ }

		void activate(UnitInfoPtr u)
		{
			unit = u;
		}

		TaskStatus::Type tick()
		{
			if (unit->isDead()) {
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
		UnitInfoPtr	unit;
};

MicroTaskPtr createBuildObserver()
{
	return MicroTaskPtr(new BuildObserverTask());
} 
