#pragma once

#include "base-task.h"
#include <BWAPI.h>

class TechObserverTask : public BaseTask
{
	public:
		TechObserverTask()
		{ }

		void activate(BWAPI::Unit* u)
		{
			unit = u;
		}

		TaskStatus::Type tick()
		{
			if (!unit->exists()) {
				return failed(unit);
			} else if (unit->isResearching()) {
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

MicroTask createTechObserver()
{
	MicroTaskData data(new TechObserverTask());
	return MicroTask(MicroTaskEnum::TechObserver, data);
} 
