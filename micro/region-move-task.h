#pragma once

#include "base-task.h"
#include <BWAPI.h>

class RegionMoveTask : public BaseTask
{
	public:
		RegionMoveTask(const BWAPI::Position& t) : BaseTask(MicroTaskEnum::RegionMove), target(t)
		{ }

		void activate(UnitInfoPtr u)
		{
			unit = u;
			lastcommandframe = -1;
		}

		TaskStatus::Type tick()
		{
			if (unit == NULL)
				return TaskStatus::failed;
				
			auto pos = unit->getPosition();
			BWAPI::Broodwar->drawLineMap(pos.x(), pos.y(), target.x(), target.y(), BWAPI::Colors::Red);

			if (lastcommandframe < 0) {
				unit->get()->rightClick(target);
				lastcommandframe = currentFrame();
			} else if (reachedTarget()) {
				return completed(unit);
			} else if (lastcommandframe + latencyFrames() > currentFrame()) {
				// DO NOTHING!
			} else if (!unit->get()->isMoving()) {
				unit->get()->rightClick(target);
				lastcommandframe = currentFrame();
			}
			return TaskStatus::running;
		}

	protected:
		UnitInfoPtr		unit;
		BWAPI::Position	target;
		int				lastcommandframe;

		bool reachedTarget() const
		{
			return (unit->getPosition().getDistance(target) < 128.);
		}
};

MicroTaskPtr createRegionMove(const BWAPI::Position& target)
{
	return MicroTaskPtr(new RegionMoveTask(target));
}

MicroTaskPtr createChokepointMove(ChokepointInfoPtr point, RegionInfoPtr region)
{
	return createRegionMove(point->getWaitingPosition(region));
}
