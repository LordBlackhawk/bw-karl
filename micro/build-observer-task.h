#pragma once

#include "base-task.h"
#include <BWAPI.h>

class BuildObserverTask : public BaseTask
{
	public:
		BuildObserverTask()
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

MicroTask createBuildObserver()
{
	MicroTaskData data(new BuildObserverTask());
	return MicroTask(MicroTaskEnum::BuildObserver, data);
} 
