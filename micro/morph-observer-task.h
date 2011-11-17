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

		void tick(BWAPI::Unit* u)
		{
			unit = u;
			if (!unit->exists()) {
				failed(unit);
			} else if (unit->isMorphing()) {
				// WAIT ...
			} else {
				completed(unit);
			}
			// TODO: isUnderAttack?
		}

	protected:
		BWAPI::Unit*		unit;
};

MicroTask createMorphObserver()
{
	MicroTaskData data(new MorphObserverTask());
	return MicroTask(MicroTaskEnum::MorphObserver, data);
} 
