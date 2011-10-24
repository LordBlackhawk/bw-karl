#ifndef BUILDNEARTASK_h
#define BUILDNEARTASK_h

#include "task.h"
#include "buildattask.h"
#include "BuildingPlacer.h"

class BuildNearTask : public BuildAtTask
{
public:
	static BuildingPlacer* placer;
	TilePosition near_pos;

public:
	BuildNearTask(Task* o, int p, UnitType ut, TilePosition near, bool ew = false)
		: BuildAtTask(o, p, ut, getBuildLocationNear(near, ut), ew),
		  near_pos(near)
	{ }

	static const char* getNameStatic()
	{
		return "BuildNear";
	}

	virtual const char* getName() const
	{
		return getNameStatic();
	}

	virtual void computeActions(int fcount)
	{
		BuildAtTask::computeActions(fcount);

		if (fcount % 101 == 29) {
			if (status == TaskStatus::WaitingForLocation) {
				tileposition = getBuildLocationNear(near_pos, unittype);
				if (tileposition == TilePositions::None)
					return;
				checkStatus();
			}
		}
	}

private:
	static TilePosition getBuildLocationNear(TilePosition near, UnitType ut)
	{
		if (placer == NULL)
			placer = new BuildingPlacer;
		TilePosition res = placer->getBuildLocationNear(near, ut);
		if (res == TilePositions::None)
			return TilePositions::Unknown;

		placer->reserveTiles(res, ut.tileWidth(), ut.tileHeight());
		return res;
	}
};

BuildingPlacer* BuildNearTask::placer = NULL;

#endif