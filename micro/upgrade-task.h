#pragma once

#include "base-task.h"
#include <BWAPI.h>

class UpgradeTask : BaseTask
{
	public:
		UpgradeTask(const BWAPI::UpgradeType t) : gt(t)
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
				unit->upgrade(gt);
				lastcommandframe = currentFrame();
			} else if (unit->isUpgrading()) {
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
		BWAPI::UpgradeType	gt;
		int					lastcommandframe;
};

MicroTask createUpgrade(const BWAPI::UpgradeType& gt)
{
	MicroTaskData data(new UpgradeTask(gt));
	return MicroTask(MicroTaskEnum::Upgrade, data);
} 
