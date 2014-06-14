#pragma once

#include "base-task.h"

#include "informations/informations.h"

#include <BWAPI.h>
#include <BWTA.h>

class LongMoveTask : public BaseTask
{
	public:
		LongMoveTask(const BWAPI::Position& t) : BaseTask(MicroTaskEnum::LongMove), target(t)
		{
			targetregion = InformationKeeper::instance().getRegion(target);
			assert(targetregion != NULL);
		}
		
		void activate(UnitInfoPtr u)
		{
			unit  = u;
			stask = MicroTaskPtr();
		}

		TaskStatus::Type tick()
		{
			if (stask == NULL) {
				auto res = updateWay();
				if (res != TaskStatus::running)
					return res;
			}

			TaskStatus::Type type = stask->tick();
			switch (type)
			{
				case TaskStatus::completed:
					return updateWay();
				
				case TaskStatus::running:
					return TaskStatus::running;

				default:
					return failed(unit);
			}
		}

	protected:
		UnitInfoPtr						unit;
		BWAPI::Position					target;
		RegionInfoPtr 					targetregion;
		ChokepointInfoPtr				nextwaypoint;
		RegionInfoPtr					nextregion;

		MicroTaskPtr					stask;

		TaskStatus::Type updateWay()
		{
			assert(unit != NULL);
			BWAPI::TilePosition pos = unit->getTilePosition();
			RegionInfoPtr currentregion = InformationKeeper::instance().getRegion(pos);
			assert(currentregion != NULL);
			
			if (reachedTarget()) {
				return completed(unit);
			} else if (currentregion == targetregion) {
				stask = createRegionMove(target);
				unit->pushTask(stask);
			} else if ((nextregion == NULL) || (nextregion == currentregion)) {
				InformationKeeper::instance().getShortestWay(pos, BWAPI::TilePosition(target), nextwaypoint);
				if (nextwaypoint == NULL) {
					LOG1 << "Error in LongMoveTask: No way found!";
					return failed(unit);
				}
				nextregion = nextwaypoint->getOtherRegion(currentregion);
				stask = createRegionMove(nextwaypoint->getWaitingPosition(currentregion));
				unit->pushTask(stask);
			} else {
				stask = createChokepointMove(nextwaypoint, nextregion);
				unit->pushTask(stask);
			}
			return TaskStatus::running;
		}

		bool reachedTarget() const
		{
			assert(unit != NULL);
			return (unit->getPosition().getDistance(target) < 128.);
		}
};

MicroTaskPtr createLongMove(const BWAPI::Position& target)
{
	return MicroTaskPtr(new LongMoveTask(target));
}
