#pragma once

#include "base-task.h"
#include <BWAPI.h>
#include <BWTA.h>

class GatherGasTask : public BaseTask
{
	public:
		GatherGasTask(BWAPI::Unit* g) : geyser(g)
		{ }

		void activate(BWAPI::Unit* unit)
		{
			worker.insert(unit);
			unit->rightClick(geyser);
		}
		
		void deactive(BWAPI::Unit* unit)
		{
			worker.erase(unit);
		}

		void tick(BWAPI::Unit* /*unit*/)
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
			completed(unit);
		}
		
		BWAPI::Position getPosition() const
		{
			return geyser->getPosition();
		}

	protected:
		std::set<BWAPI::Unit*>	worker;
		BWAPI::Unit* geyser;
};
