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

		TaskStatus::Type tick()
		{
			if (lastcommandframe < 0) {
				unit->train(ut);
				lastcommandframe = currentFrame();
			} else if (unit->getLastCommand().getType() == BWAPI::UnitCommandTypes::Train) {
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

MicroTask createTrain(const BWAPI::UnitType& ut)
{
	MicroTaskData data(new TrainTask(ut));
	return MicroTask(MicroTaskEnum::Train, data);
} 
