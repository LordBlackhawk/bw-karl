#pragma once

#include "base-task.h"
#include <BWAPI.h>

class TechTask : public BaseTask
{
	public:
		TechTask(const BWAPI::TechType t) : BaseTask(MicroTaskEnum::Tech), tt(t)
		{ }

		void activate(UnitInfoPtr u)
		{
			unit = u;
			lastcommandframe = -1;
		}

		TaskStatus::Type tick()
		{
			if (lastcommandframe < 0) {
				unit->get()->research(tt);
				lastcommandframe = currentFrame();
			} else if (unit->get()->isResearching()) {
				return completed(unit);
			} else if (lastcommandframe + latencyFrames() > currentFrame()) {
				// WAIT ...
			} else {
				// DO ANALYSIS, TRYAGAIN OR FAIL ...
				return failed(unit);
			}
			
			return TaskStatus::running;
		}

	protected:
		UnitInfoPtr			unit;
		BWAPI::TechType		tt;
		int					lastcommandframe;
};

MicroTaskPtr createTech(const BWAPI::TechType& tt)
{
	return MicroTaskPtr(new TechTask(tt));
} 
