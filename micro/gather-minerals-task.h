#pragma once

#include "base-task.h"
#include <BWAPI.h>
#include <BWTA.h>

class GatherMineralsTask : BaseTask
{
	public:
		GatherMineralsTask(BWTA::BaseLocation* l) : location(l)
		{ }

		void activate(BWAPI::Unit* unit)
		{
			worker.insert(u);
			unit->rightClick(getMineral());
		}
		
		void deactive(BWAPI::Unit* unit)
		{
			worker.erase(u);
		}

		void tick(BWAPI::Unit* unit)
		{ }

	protected:
		std::set<BWAPI::Unit*>	worker;
		BWTA::BaseLocation*		location;
		
		BWAPI::Unit* getMineral() const
		{
			return /*TODO*/;
		}
};
