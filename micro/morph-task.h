#pragma once

#include "base-task.h"
#include <BWAPI.h>

class MorphTask : public BaseTask
{
	public:
		MorphTask(const BWAPI::UnitType t) : ut(t)
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
				unit->morph(ut);
				lastcommandframe = currentFrame();
			} else if (unit->isMorphing()) {
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
		BWAPI::UnitType		ut;
		int					lastcommandframe;
};

MicroTask createMorph(const BWAPI::UnitType& ut)
{
	MicroTaskData data(new MorphTask(ut));
	return MicroTask(MicroTaskEnum::Morph, data);
} 
