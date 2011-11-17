#pragma once

#include "base-task.h"
#include <BWAPI.h>

class RegionMoveTask : public BaseTask
{
	public:
		RegionMoveTask(const BWAPI::Position& t) : target(t)
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
				unit->rightClick(target);
				lastcommandframe = currentFrame();
			} else if (reachedTarget()) {
				completed(unit);
			} else if (lastcommandframe + latencyFrames() > currentFrame()) {
				// DO NOTHING!
			} else if (!unit->isMoving()) {
				unit->rightClick(target);
				lastcommandframe = currentFrame();
			}
		}

	protected:
		BWAPI::Unit*	unit;
		BWAPI::Position	target;
		int				lastcommandframe;

		bool reachedTarget() const
		{
			return (unit->getPosition().getDistance(target) < 64.);
		}
};

MicroTask createRegionMove(const BWAPI::Position& target)
{
	MicroTaskData data(new RegionMoveTask(target));
	return MicroTask(MicroTaskEnum::RegionMove, data);
}
