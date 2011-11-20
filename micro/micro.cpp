#include "micro.h"
#include "micro-checkpoints.h"
#include "micro-task-impl.h"
#include "micro-task-manager.h"
#include "unit-distributer.h"
#include "worker-manager.h"
#include "scout-manager.h"


void Micro::clear()
{
	MicroTaskManager::instance().clear();
	WorkerManager::instance().clear();
	ScoutManager::instance().clear();
}

void Micro::prepareMap()
{
	ScoutManager::instance().prepareMap();
}

void Micro::pretick()
{
	BWAPI::Player* self = BWAPI::Broodwar->self();
	for (auto event : BWAPI::Broodwar->getEvents())
	{
		switch (event.getType())
		{
			case BWAPI::EventType::UnitCreate:
				if (event.getUnit()->getPlayer() == self) {
					std::clog << "Adding " << event.getUnit()->getType().getName() << " to MicroTaskHandler...\n";
					MicroTaskManager::instance().onUnitAdded(event.getUnit());
				}
				break;
			case BWAPI::EventType::UnitDestroy:
				if (event.getUnit()->getPlayer() == self)
					MicroTaskManager::instance().onUnitDestroyed(event.getUnit());
				break;
			default:
				break;
		}
	}
}

void Micro::tick()
{
	UnitDistributer::instance().tick();
	WorkerManager::instance().tick();
	MicroTaskManager::instance().tick();
	ScoutManager::instance().tick();
}



