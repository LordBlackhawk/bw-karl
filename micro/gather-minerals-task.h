#pragma once

#include "base-task.h"
#include "utils/random-chooser.h"
#include <BWAPI.h>
#include <BWTA.h>

class GatherMineralsTask : public BaseTask
{
	public:
		GatherMineralsTask(BaseLocationInfoPtr l) : location(l)
		{ }

		void activate(BWAPI::Unit* unit)
		{
			if (unit->getType().isWorker()) {
				worker.insert(unit);
				unit->rightClick(getMineral());
			} else {
				failed(unit);
			}
		}
		
		void deactivate(BWAPI::Unit* unit)
		{
			worker.erase(unit);
		}

		TaskStatus::Type tick()
		{
			for (auto unit : worker)
				if (unit->isIdle()) {
					if (unit->getType().isWorker())
						unit->rightClick(getMineral());
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
		std::set<BWAPI::Unit*>	worker;
		BaseLocationInfoPtr		location;
		
		BWAPI::Unit* getMineral() const
		{
			// TODO BETTER:
			return getRandomSomething(location->get()->getMinerals());
		}
};
