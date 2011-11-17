#pragma once

#include "base-task.h"
#include <BWAPI.h>

class BuildObserverTask : BaseTask
{
	public:
		BuildObserverTask()
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

MicroTask createBuildObserver()
{
	MicroTaskData data(new BuildObserverTask());
	return MicroTask(MicroTaskEnum::BuildObserver, data);
} 
