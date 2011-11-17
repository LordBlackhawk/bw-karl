#pragma once

#include "base-task.h"
#include <BWAPI.h>
#include <BWTA.h>

class LongMoveTask : public BaseTask
{
	public:
		LongMoveTask(const BWAPI::Position& t) : target(t)
		{ }
		
		void activate(BWAPI::Unit* u)
		{
			unit = u;
			updateWay();
		}

		void tick(BWAPI::Unit* u)
		{
			unit = u;
			if (reachedTarget()) {
				completed(unit);
			} else if (nextwaypoint == NULL) {
				stask = createRegionMove(target);
				subtask(unit, stask);
			} else if (nearNextWaypoint()) {
				stask = createChokepointMove();
				subtask(unit, stask);
			} else {
				stask = createRegionMove(getChokepointWaitingPosition());
				subtask(unit, stask);
			}
		}

	protected:
		BWAPI::Unit*					unit;
		BWAPI::Position					target;
		BWTA::Chokepoint*				nextwaypoint;

		MicroTask						stask;

		void calculatedWay()
		{
			BWAPI::TilePosition pos  = unit->getTilePosition();
			BWTA::Region startregion = BWTA::getRegion(pos);
			BWTA::Region endregion   = BWTA::getRegion(target);
			if (startregion == endregion) {
				nextwaypoint = NULL;
				return;
			}
			nextwaypoint = PathFinder::findBestWaypoint(startregion, endregion);
		}

		bool reachedTarget() const
		{
			return /*TODO*/;
		}

		bool nearNextWaypoint() const
		{
			return /*TODO*/;
		}

		BWAPI::Position getChokepointWaitingPosition() const
		{
			return /*TODO*/;
		}
};

MicroTask createLongMove(const BWAPI::Position& target)
{
	MicroTaskData data(new LongMoveTask(target));
	return MicroTask(MicroTaskEnum::LongMove, data);
}