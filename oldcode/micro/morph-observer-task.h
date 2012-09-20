#pragma once

#include "base-task.h"
#include <BWAPI.h>

class MorphObserverTask : public BaseTask
{
	public:
		MorphObserverTask() : BaseTask(MicroTaskEnum::MorphObserver)
		{ }

		void activate(UnitInfoPtr u)
		{
			unit = u;
		}

		TaskStatus::Type tick()
		{
			if (!unit->get()->exists()) {
				return failed(unit);
			} else if (unit->get()->isMorphing()) {
				// WAIT ...
			} else if (unit->get()->isIdle()) {
				return completed(unit);
			}
			// TODO: isUnderAttack?
			return TaskStatus::running;
		}

	protected:
		UnitInfoPtr		unit;
};

MicroTaskPtr createMorphObserver()
{
	return MicroTaskPtr(new MorphObserverTask());
} 
