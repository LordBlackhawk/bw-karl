#pragma once

#include "base-task.h"

#include "informations/informations.h"

#include <BWAPI.h>
#include <BWTA.h>

class LongMoveTask : public BaseTask
{
	public:
		LongMoveTask(const BWAPI::Position& t) : target(t)
		{
			targetregion = InformationKeeper::instance().getRegion(target);
		}
		
		void activate(BWAPI::Unit* u)
		{
			unit = u;
			updateWay();
		}

		TaskStatus::Type tick()
		{
			if (subtask.empty())
				return reachedTarget() ? completed() : failed();

			TaskStatus::Type type = subtask.tick();
			switch (type)
			{
				case TaskStatus::completed:
					return updateWay();
				
				case TaskStatus::running:
				case TaskStatus::failed:
					return type;
			}
		}

	protected:
		BWAPI::Unit*					unit;
		BWAPI::Position					target;
		RegionInfoPtr 					targetregion;
		ChokepointInfoPtr				nextwaypoint;
		RegionInfoPtr					nextregion;

		MicroTask						subtask;

		TaskStatus::Type updateWay()
		{
			BWAPI::TilePosition pos = unit->getTilePosition();
			RegionInfoPtr currentregion = InformationKeeper::instance().getRegion(pos);
			
			if (reachedTarget()) {
				return completed(unit);
			} else if (currentregion == targetregion) {
				stask = createRegionMove(target);
				subtask(unit, stask);
			} else if ((nextregion.use_count() == 0) || (nextregion == currentregion)) {
				InformationKeeper::instance().getShortestWay(pos, target, nextwaypoint);
				nextregion = nextwaypoint->getOtherRegion(currentregion);
				stask = createRegionMove(nextwaypoint->getWaitingPosition(currentregion));
				subtask(unit, stask);
			} else {
				stask = createChokepointMove(nextwaypoint, nextregion);
				subtask(unit, stask);
			}
			return TaskStatus::running;
		}

		bool reachedTarget() const
		{
			return (unit->getPosition().getDistance(target) < 128.);
		}
};

MicroTask createLongMove(const BWAPI::Position& target)
{
	MicroTaskData data(new LongMoveTask(target));
	return MicroTask(MicroTaskEnum::LongMove, data);
}

/*
MicroTask createLongMove(const BWAPI::Position& target)
{
	return createRegionMove(target);
}
*/
