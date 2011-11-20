#pragma once

#include "base-task.h"
#include <BWAPI.h>

class MorphObserverTask : public BaseTask
{
	public:
		MorphObserverTask()
		{ }

		void activate(BWAPI::Unit* u)
		{
			unit = u;
		}

		TaskStatus::Type tick()
		{
			if (!unit->exists()) {
				return failed(unit);
			} else if (unit->isMorphing()) {
				// WAIT ...
			} else if (unit->isIdle()) {
				return completed(unit);
			}
			// TODO: isUnderAttack?
			return TaskStatus::running;
		}

	protected:
		BWAPI::Unit*		unit;
};

MicroTask createMorphObserver()
{
	MicroTaskData data(new MorphObserverTask());
	return MicroTask(MicroTaskEnum::MorphObserver, data);
} 
