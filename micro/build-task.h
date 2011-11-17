#pragma once

#include "base-task.h"
#include <BWAPI.h>

class BuildTask : public BaseTask
{
	public:
		BuildTask(const BWAPI::UnitType t, const BWAPI::TilePosition& p) : ut(t), pos(p)
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
				unit->build(pos, ut);
				lastcommandframe = currentFrame();
			} else if (unit->getLastCommand().getType() == BWAPI::UnitCommandTypes::Build) {
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
		BWAPI::UnitType		ut;
		BWAPI::TilePosition	pos;
		int					lastcommandframe;
};

MicroTask createBuild(const BWAPI::UnitType& ut, const BWAPI::TilePosition& pos)
{
	MicroTaskData data(new BuildTask(ut, pos));
	return MicroTask(MicroTaskEnum::Build, data);
} 
