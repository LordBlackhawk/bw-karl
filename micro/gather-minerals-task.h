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
			if (unit->getType().isWorker()) {
				worker.insert(unit);
				unit->rightClick(getMineral());
			} else {
				failed(unit);
			}
		}
		
		void deactive(BWAPI::Unit* unit)
		{
			worker.erase(unit);
		}

		TaskStatus::Type tick()
		{
			for (auto unit : worker)
				if (unit->isIdle())
					unit->rightClick(getMineral());
			return TaskStatus::running;
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
