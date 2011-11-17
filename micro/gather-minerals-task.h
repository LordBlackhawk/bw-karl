#pragma once

#include "base-task.h"
#include "utils/random-chooser.h"
#include <BWAPI.h>
#include <BWTA.h>

class GatherMineralsTask : public BaseTask
{
	public:
		GatherMineralsTask(BWTA::BaseLocation* l) : location(l)
		{ }

		void activate(BWAPI::Unit* unit)
		{
			worker.insert(unit);
			unit->rightClick(getMineral());
		}
		
		void deactive(BWAPI::Unit* unit)
		{
			worker.erase(unit);
		}

		void tick(BWAPI::Unit* unit)
		{
			if (unit->isIdle())
				unit->rightClick(getMineral());
		}
		
		BWAPI::Position getPosition() const
		{
			return getMineral()->getPosition();
		}

	protected:
		std::set<BWAPI::Unit*>	worker;
		BWTA::BaseLocation*		location;
		
		BWAPI::Unit* getMineral() const
		{
			// TODO BETTER:
			return getRandomSomething(location->getMinerals());
		}
};
