#pragma once

#include "base-task.h"
#include "bwapi-helper.h"
#include <BWAPI.h>

class BuildTask : public BaseTask
{
	public:
		enum { max_tries = 200 };
		
		BuildTask(const BWAPI::UnitType t, const BWAPI::TilePosition& p) : ut(t), pos(p)
		{ }

		void activate(BWAPI::Unit* u)
		{
			unit = (u->getType().isWorker()) ? u : NULL;
			lastcommandframe = -1;
			tries = 0;
		}

		TaskStatus::Type tick()
		{
			if (unit == NULL)
				return failed(unit);

			if (lastcommandframe < 0) {
				if (!unit->build(pos, ut)) {
					unit->rightClick(getBuildingCenter(pos, ut));
					++tries;
					if (tries > max_tries)
						return failed(unit);
				} else {
					lastcommandframe = currentFrame();
				}
			} else if (isComplete()) {
				//if (unit->getType().getRace() == BWAPI::Races::Zerg)
				//	return completedAndClearAll(unit);
				return completed(unit);
			} else if (lastcommandframe + latencyFrames() + 1 > currentFrame()) {
				// WAIT ...
			} else if (unit->isIdle()) {
				// DO ANALYSIS, TRYAGAIN OR FAIL ...
				//failed(unit);
				tries = 0;
				lastcommandframe = -1;
			}
			return TaskStatus::running;
		}
		
		bool isComplete() const
		{
			if (ut.getRace() != BWAPI::Races::Zerg) {
				return (unit->getLastCommand().getType() == BWAPI::UnitCommandTypes::Build);
			} else if (ut != BWAPI::UnitTypes::Zerg_Extractor) {
				return (unit->getType() == ut);
			} else {
				return (!unit->exists());
			}
		}

	protected:
		BWAPI::Unit*		unit;
		BWAPI::UnitType		ut;
		BWAPI::TilePosition	pos;
		int					lastcommandframe;
		int					tries;
};

MicroTask createBuild(const BWAPI::UnitType& ut, const BWAPI::TilePosition& pos)
{
	MicroTaskData data(new BuildTask(ut, pos));
	return MicroTask(MicroTaskEnum::Build, data);
} 
