#pragma once

#include "micro.h"
#include "micro-checkpoints.h"
#include "micro-task-impl.h"
#include "unit-distributer.h"
#include "worker-manager.h"

void Micro::clear()
{
	MicroTaskManager::instance().clear();
	WorkerManager::instance().clear();
}

void Micro::tick()
{
	BWAPI::Player* self = Broodwar->self();
	for (auto event : Broodwar->getEvents())
	{
		switch (event->getType())
		{
			case EventType::UnitCreate:
				if (event->getUnit()->getPlayer() == self)
					MicroTaskManager::onUnitAdded(event->getUnit());
				break;
			case EventType::UnitDestroy:
				if (event->getUnit()->getPlayer() == self)
					MicroTaskManager::onUnitDestroyed(event->getUnit());
				break;
			default:
				break;
		}
	}

	UnitDistributer::instance().tick();

	MircoTaskManager::instance().onTick();
}



