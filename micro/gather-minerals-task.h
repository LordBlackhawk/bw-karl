#pragma once

#include "base-task.h"
#include "utils/random-chooser.h"
#include <BWAPI.h>
#include <BWTA.h>

class GatherMineralsTask : public BaseTask
{
	public:
		GatherMineralsTask(BaseLocationInfoPtr l) : BaseTask(MicroTaskEnum::GatherMinerals), location(l)
		{ }

		virtual void activate(UnitInfoPtr unit)
		{
			if (unit->getType().isWorker()) {
				worker.insert(unit);
				unit->get()->rightClick(getMineral());
			} else {
				failed(unit);
			}
		}
		
		virtual void deactivate(UnitInfoPtr unit)
		{
			worker.erase(unit);
		}

		virtual TaskStatus::Type tick()
		{
			for (auto unit : worker)
				if (unit->get()->isIdle()) {
					if (unit->getType().isWorker())
						unit->get()->rightClick(getMineral());
					else
						LOG1 << "GatherMineralsTask has none worker unit of type '" << unit->getType().getName() << "' in list!";
				}
			return TaskStatus::running;
		}
		
		BWAPI::Position getPosition() const
		{
			return location->getPosition();
		}

	protected:
		std::set<UnitInfoPtr>	worker;
		BaseLocationInfoPtr		location;
		
		BWAPI::Unit* getMineral() const
		{
			// TODO BETTER:
			return getRandomSomething(location->get()->getMinerals());
		}
};
