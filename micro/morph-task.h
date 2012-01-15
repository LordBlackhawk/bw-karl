#pragma once

#include "base-task.h"
#include <BWAPI.h>

class MorphTask : public BaseTask
{
	public:
		enum { max_tries = 200 };
		
		MorphTask(const BWAPI::UnitType t) : BaseTask(MicroTaskEnum::Morph), ut(t)
		{ }

		void activate(UnitInfoPtr u)
		{
			unit = u;
			lastcommandframe = -1;
			tries = 0;
		}

		TaskStatus::Type tick()
		{
			if (unit == NULL)
				return TaskStatus::failed;

			if (lastcommandframe < 0) {
				if (!unit->get()->morph(ut)) {
					++tries;
					if (tries > max_tries)
						return failed(unit);
				} else {
					lastcommandframe = currentFrame();
				}
			} else if (checkStarted()) {
				LOG2 << "Morphing " << ut.getName() << " started.";
				return completed(unit);
			} else if (lastcommandframe + latencyFrames() > currentFrame()) {
				// WAIT ...
			} else {
				// DO ANALYSIS, TRYAGAIN OR FAIL ...
				return failed(unit);
			}
			return TaskStatus::running;
		}
		
		bool checkStarted() const
		{
			if (ut.isBuilding())
				return unit->getType() == ut;
			else
				return unit->get()->isMorphing();
		}

	protected:
		UnitInfoPtr			unit;
		BWAPI::UnitType		ut;
		int					lastcommandframe;
		int					tries;
};

MicroTaskPtr createMorph(const BWAPI::UnitType& ut)
{
	return MicroTaskPtr(new MorphTask(ut));
} 
