#pragma once

#include "base-task.h"
#include <BWAPI.h>

class TrainTask : public BaseTask
{
	public:
		TrainTask(const BWAPI::UnitType t) : BaseTask(MicroTaskEnum::Train), ut(t)
		{ }

		void activate(UnitInfoPtr u)
		{
			unit = u;
			lastcommandframe = -1;
		}

		TaskStatus::Type tick()
		{
			if (lastcommandframe < 0) {
				unit->get()->train(ut);
				lastcommandframe = currentFrame();
			} else if (unit->get()->getLastCommand().getType() == BWAPI::UnitCommandTypes::Train) {
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
		BWAPI::UnitType		ut;
		int					lastcommandframe;
};

MicroTaskPtr createTrain(const BWAPI::UnitType& ut)
{
	return MicroTaskPtr(new TrainTask(ut));
} 
