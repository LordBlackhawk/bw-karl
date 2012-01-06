#include "micro.h"
#include "micro-checkpoints.h"
#include "micro-task-impl.h"
#include "micro-task-manager.h"
#include "unit-distributer.h"
#include "worker-manager.h"
#include "scout-manager.h"

#include "informations/informations.h"

void Micro::clear()
{
	InformationKeeper::instance().clear();
	MicroTaskManager::instance().clear();
	WorkerManager::instance().clear();
	ScoutManager::instance().clear();
}

void Micro::prepareMap()
{
	InformationKeeper::instance().prepareMap();
	BuildingPlacer::instance().init();
	ScoutManager::instance().prepareMap();
}

void Micro::pretick()
{
	InformationKeeper::instance().pretick();
	BWAPI::Player* self = BWAPI::Broodwar->self();
	for (auto event : BWAPI::Broodwar->getEvents())
	{
		switch (event.getType())
		{
			case BWAPI::EventType::UnitCreate:
			{
				BWAPI::Unit* unit = event.getUnit();
				if (unit->getPlayer() == self) {
					LOG2 << "Adding " << unit->getType().getName() << " to MicroTaskHandler...";
					MicroTaskManager::instance().onUnitAdded(unit);
				}
				break;
			}
			case BWAPI::EventType::UnitDestroy:
			{
				BWAPI::Unit* unit = event.getUnit();
				if (unit->getPlayer() == self)
					MicroTaskManager::instance().onUnitDestroyed(unit);
				break;
			}
			default:
				break;
		}
	}
	
	WorkerManager::instance().updateBases();
}

void Micro::tick()
{
	InformationKeeper::instance().tick();
	UnitDistributer::instance().tick();
	WorkerManager::instance().tick();
	MicroTaskManager::instance().tick();
	ScoutManager::instance().tick();
}



