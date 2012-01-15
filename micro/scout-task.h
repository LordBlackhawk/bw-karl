#pragma once

#include "base-task.h"
#include <BWAPI.h>
#include <BWTA.h>

class ScoutTask : public BaseTask
{
	public:
		ScoutTask(BaseLocationInfoPtr l) : BaseTask(MicroTaskEnum::Scout), location(l)
		{ }

		void activate(UnitInfoPtr u)
		{
			unit    = u;
			stask = createLongMove(location->getPosition());
			unit->pushTask(stask);
		}

		TaskStatus::Type tick()
		{
			TaskStatus::Type res = stask->tick();
			switch (res)
			{
				case TaskStatus::completed:
					return completed(unit);
				
				case TaskStatus::failed:
					return failed(unit);
				
				default:
					return res;
			}
		}

	protected:
		UnitInfoPtr			unit;
		BaseLocationInfoPtr	location;
		MicroTaskPtr		stask;
};

MicroTaskPtr createScout(BaseLocationInfoPtr loc)
{
	return MicroTaskPtr(new ScoutTask(loc));
} 
