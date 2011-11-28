#pragma once

#include "base-task.h"
#include <BWAPI.h>

class MorphTask : public BaseTask
{
	public:
		enum { max_tries = 200 };
		
		MorphTask(const BWAPI::UnitType t) : ut(t)
		{ }

		void activate(BWAPI::Unit* u)
		{
			unit = u;
			lastcommandframe = -1;
			tries = 0;
		}

		TaskStatus::Type tick()
		{
			if (unit == NULL)
				return TaskStatus::failed;

			if (lastcommandframe < 0) {
				if (!unit->morph(ut)) {
					++tries;
					if (tries > max_tries)
						return failed(unit);
				} else {
					lastcommandframe = currentFrame();
				}
			} else if (unit->isMorphing()) {
				LOG2 << "Morphing " << ut.getName() << " started.";
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
		int					tries;
};

MicroTask createMorph(const BWAPI::UnitType& ut)
{
	MicroTaskData data(new MorphTask(ut));
	return MicroTask(MicroTaskEnum::Morph, data);
} 
