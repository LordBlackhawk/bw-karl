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

		TaskStatus::Type tick()
		{
			if (lastcommandframe < 0) {
				unit->rightClick(target);
				lastcommandframe = currentFrame();
			} else if (reachedTarget()) {
				return completed(unit);
			} else if (lastcommandframe + latencyFrames() > currentFrame()) {
				// DO NOTHING!
			} else if (!unit->isMoving()) {
				unit->rightClick(target);
				lastcommandframe = currentFrame();
			}
			return TaskStatus::running;
		}

	protected:
		BWAPI::Unit*	unit;
		BWAPI::Position	target;
		int				lastcommandframe;

		bool reachedTarget() const
		{
			return (unit->getPosition().getDistance(target) < 128.);
		}
};

MicroTask createRegionMove(const BWAPI::Position& target)
{
	MicroTaskData data(new RegionMoveTask(target));
	return MicroTask(MicroTaskEnum::RegionMove, data);
}
