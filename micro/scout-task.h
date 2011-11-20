#pragma once

#include "base-task.h"
#include <BWAPI.h>
#include <BWTA.h>

class ScoutTask : public BaseTask
{
	public:
		BuildTask(const BWTA::BaseLocation* l) : location(l)
		{ }

		void activate(BWAPI::Unit* u)
		{
			unit    = u;
			subtask = createLongMove(location->getPosition());
		}

		TaskStatus::Type tick()
		{
			return subtask.tick();
		}

	protected:
		BWAPI::Unit*		unit;
		BWTA::BaseLocation*	location;
		MicroTask			subtask;
};

MicroTask createScout(const BWTA::BaseLocation* loc)
{
	MicroTaskData data(new ScoutTask(loc));
	return MicroTask(MicroTaskEnum::Scout, data);
} 
