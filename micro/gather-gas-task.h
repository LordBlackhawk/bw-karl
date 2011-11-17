#pragma once

#include "base-task.h"
#include <BWAPI.h>
#include <BWTA.h>

class GatherGasTask : BaseTask
{
	public:
		GatherGasTask(BWAPI::Unit* g) : geyser(g)
		{ }

		void activate(BWAPI::Unit* unit)
		{
			worker.insert(u);
			unit->rightClick(geyser);
		}
		
		void deactive(BWAPI::Unit* unit)
		{
			worker.erase(u);
		}

		void tick(BWAPI::Unit* unit)
		{ }

		int workercount() const
		{
			return worker.size();
		}

		void giveBackWorker()
		{
			if (worker.size() < 1)
				return;
			BWAPI::Unit* unit = *worker.begin();
			complete(unit);
		}

	protected:
		std::set<BWAPI::Unit*>	worker;
		BWAPI::Unit* geyser;
};
