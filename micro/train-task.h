#pragma once

#include "base-task.h"
#include <BWAPI.h>

class TrainTask : public BaseTask
{
	public:
		TrainTask(const BWAPI::UnitType t) : ut(t)
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
				unit->train(ut);
				lastcommandframe = currentFrame();
			} else if (unit->getLastCommand().getType() == BWAPI::UnitCommandTypes::Train) {
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

MicroTask createTrain(const BWAPI::UnitType& ut)
{
	MicroTaskData data(new TrainTask(ut));
	return MicroTask(MicroTaskEnum::Train, data);
} 
