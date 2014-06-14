#pragma once

#include "base-task.h"
#include "bwapi-helper.h"
#include <BWAPI.h>

class BuildTask : public BaseTask
{
	public:
		enum { max_tries = 200 };
		
		BuildTask(const BWAPI::UnitType t, const BWAPI::TilePosition& p) : BaseTask(MicroTaskEnum::Build), ut(t), pos(p)
		{ }

		void activate(UnitInfoPtr u)
		{
			unit = (u->getType().isWorker()) ? u : UnitInfoPtr();
			lastcommandframe = -1;
			tries = 0;
		}

		TaskStatus::Type tick()
		{
			if (unit == NULL)
				return failed(unit);

			BWAPI::Unit* direct = unit->get();
			if (lastcommandframe < 0) {
				if (!direct->build(pos, ut)) {
					direct->rightClick(getBuildingCenter(pos, ut));
					++tries;
					if (tries > max_tries)
						return failed(unit);
				} else {
					lastcommandframe = currentFrame();
				}
			} else if (isComplete()) {
				return completed(unit);
			} else if (lastcommandframe + latencyFrames() + 1 > currentFrame()) {
				// WAIT ...
			} else if (direct->isIdle()) {
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
				return (unit->get()->getLastCommand().getType() == BWAPI::UnitCommandTypes::Build);
			} else if (ut != BWAPI::UnitTypes::Zerg_Extractor) {
				return (unit->getType() == ut);
			} else {
				return unit->isDead();
			}
		}

	protected:
		UnitInfoPtr			unit;
		BWAPI::UnitType		ut;
		BWAPI::TilePosition	pos;
		int					lastcommandframe;
		int					tries;
};

MicroTaskPtr createBuild(const BWAPI::UnitType& ut, const BWAPI::TilePosition& pos)
{
	return MicroTaskPtr(new BuildTask(ut, pos));
} 
