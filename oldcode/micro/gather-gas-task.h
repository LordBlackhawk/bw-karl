#pragma once

#include "base-task.h"

#include "utils/debug.h"

#include <BWAPI.h>
#include <BWTA.h>

class GatherGasTask : public BaseTask
{
	public:
		GatherGasTask(UnitInfoPtr g) : BaseTask(MicroTaskEnum::GatherGas), geyser(g)
		{ }

		void activate(UnitInfoPtr unit)
		{
			if (geyser == NULL)
				return;
			BWAPI::UnitType ut = unit->getType();
			if (ut.isWorker()) {
				LOG2 << "GatherGasTask: Worker added.";
				worker.insert(unit);
				unit->get()->rightClick(geyser->get());
			} else if (ut.isRefinery()) {
				geyser = unit;
			} else {
				failed(unit);
			}
		}
		
		void deactivate(UnitInfoPtr unit)
		{
			if (geyser == NULL)
				return;

			if (unit == geyser) {
			    geyser = UnitInfoPtr();
				for (auto it : worker)
					completed(it);
			} else {
				worker.erase(unit);
			}
		}

		TaskStatus::Type tick()
		{
			if (geyser == NULL)
				return TaskStatus::failed;
				
			for (auto it : worker)
				if (!it->get()->isGatheringGas())
					it->get()->rightClick(geyser->get());
			
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
			UnitInfoPtr unit = *worker.begin();
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
		std::set<UnitInfoPtr>	worker;
		UnitInfoPtr geyser;
};
