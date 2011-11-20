#pragma once

#include "base-task.h"
#include <BWAPI.h>

class UpgradeTask : public BaseTask
{
	public:
		UpgradeTask(const BWAPI::UpgradeType t) : gt(t)
		{ }

		void activate(BWAPI::Unit* u)
		{
			unit = u;
			lastcommandframe = -1;
		}

		TaskStatus::Type tick()
		{
			if (lastcommandframe < 0) {
				unit->upgrade(gt);
				lastcommandframe = currentFrame();
			} else if (unit->isUpgrading()) {
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
		BWAPI::UpgradeType	gt;
		int					lastcommandframe;
};

MicroTask createUpgrade(const BWAPI::UpgradeType& gt)
{
	MicroTaskData data(new UpgradeTask(gt));
	return MicroTask(MicroTaskEnum::Upgrade, data);
} 
