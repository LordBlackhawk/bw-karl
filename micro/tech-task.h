#pragma once

#include "base-task.h"
#include <BWAPI.h>

class TechTask : public BaseTask
{
	public:
		TechTask(const BWAPI::TechType t) : tt(t)
		{ }

		void activate(BWAPI::Unit* u)
		{
			unit = u;
			lastcommandframe = -1;
		}

		TaskStatus::Type tick()
		{
			if (lastcommandframe < 0) {
				unit->research(tt);
				lastcommandframe = currentFrame();
			} else if (unit->isResearching()) {
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
		BWAPI::Unit*		unit;
		BWAPI::TechType		tt;
		int					lastcommandframe;
};

MicroTask createTech(const BWAPI::TechType& tt)
{
	MicroTaskData data(new TechTask(tt));
	return MicroTask(MicroTaskEnum::Tech, data);
} 
