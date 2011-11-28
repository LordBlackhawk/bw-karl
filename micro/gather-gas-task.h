#pragma once

#include "base-task.h"

#include "utils/debug.h"

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
			BWAPI::UnitType ut = unit->getType();
			if (ut.isWorker()) {
				LOG1 << "GatherGasTask: Worker added.";
				worker.insert(unit);
				unit->rightClick(geyser);
			} else if (ut.isRefinery()) {
				geyser = unit;
			} else {
				failed(unit);
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
			if (geyser == NULL)
				return TaskStatus::failed;
				
			for (auto it : worker)
				if (!it->isGatheringGas())
					it->rightClick(geyser);
			
			return TaskStatus::running;
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
