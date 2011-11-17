#pragma once

#include "base-task.h"
#include <BWAPI.h>

class TechObserverTask : BaseTask
{
	public:
		TechObserverTask()
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
			} else if (unit->isResearching()) {
				// WAIT ...
			} else {
				completed(unit);
			}
			// TODO: isUnderAttack?
		}

	protected:
		BWAPI::Unit*		unit;
};

MicroTask createTechObserver()
{
	MicroTaskData data(new TechObserverTask());
	return MicroTask(MicroTaskEnum::TechObserver, data);
} 
