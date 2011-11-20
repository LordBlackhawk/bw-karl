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

		TaskStatus::Type tick()
		{
			if (lastcommandframe < 0) {
				unit->morph(ut);
				lastcommandframe = currentFrame();
			} else if (unit->isMorphing()) {
				std::clog << BWAPI::Broodwar->getFrameCount() << ": Morphing " << ut.getName() << " started.\n";
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
		BWAPI::UnitType		ut;
		int					lastcommandframe;
};

MicroTask createMorph(const BWAPI::UnitType& ut)
{
	MicroTaskData data(new MorphTask(ut));
	return MicroTask(MicroTaskEnum::Morph, data);
} 
