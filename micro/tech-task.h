#pragma once

#include "base-task.h"
#include <BWAPI.h>

class TechTask : BaseTask
{
	public:
		TechTask(const BWAPI::TechType t) : tt(t)
		{ }

		void activate(BWAPI::Unit* u)
		{
			unit = u;
			lastcommandframe = -1;
		}

		void tick(BWAPI::Unit* u)
		{
			unit = u;
			if (lastcommandframe < 0) {
				unit->research(tt);
				lastcommandframe = currentFrame();
			} else if (unit->isResearching()) {
				completed(unit);
			} else if (lastcommandframe + latencyFrames() > currentFrame()) {
				// WAIT ...
			} else {
				// DO ANALYSIS, TRYAGAIN OR FAIL ...
				failed(unit);
			}
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
