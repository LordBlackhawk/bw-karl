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
			if (geyser == NULL)
				return;
			if ((unit != geyser) && unit->getType().isWorker()) {
				worker.insert(unit);
				unit->rightClick(geyser);
			}
		}
		
		void deactive(BWAPI::Unit* unit)
		{
			if (unit == geyser) {
				for (auto it : worker)
					completed(it);
				geyser = NULL;
			} else {
				worker.erase(unit);
			}
		}

		TaskStatus::Type tick()
		{
			return (geyser != NULL) ? TaskStatus::running : TaskStatus::failed;
		}

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
		
		bool isWorking() const
		{
			return (geyser != NULL);
		}

	protected:
		std::set<BWAPI::Unit*>	worker;
		BWAPI::Unit* geyser;
};
