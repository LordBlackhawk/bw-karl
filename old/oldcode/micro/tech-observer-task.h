#pragma once

#include "base-task.h"
#include <BWAPI.h>

class TechObserverTask : public BaseTask
{
	public:
		TechObserverTask() : BaseTask(MicroTaskEnum::TechObserver)
		{ }

		void activate(UnitInfoPtr u)
		{
			unit = u;
		}

		TaskStatus::Type tick()
		{
			if (!unit->get()->exists()) {
				return failed(unit);
			} else if (unit->get()->isResearching()) {
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

MicroTaskPtr createTechObserver()
{
	return MicroTaskPtr(new TechObserverTask());
} 
