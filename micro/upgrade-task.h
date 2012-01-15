#pragma once

#include "base-task.h"
#include <BWAPI.h>

class UpgradeTask : public BaseTask
{
	public:
		UpgradeTask(const BWAPI::UpgradeType t) : BaseTask(MicroTaskEnum::Upgrade), gt(t)
		{ }

		void activate(UnitInfoPtr u)
		{
			unit = u;
			lastcommandframe = -1;
		}

		TaskStatus::Type tick()
		{
			if (lastcommandframe < 0) {
				unit->get()->upgrade(gt);
				lastcommandframe = currentFrame();
			} else if (unit->get()->isUpgrading()) {
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
		BWAPI::UpgradeType	gt;
		int					lastcommandframe;
};

MicroTaskPtr createUpgrade(const BWAPI::UpgradeType& gt)
{
	return MicroTaskPtr(new UpgradeTask(gt));
} 
