#include "micro.h"
#include "micro-checkpoints.h"
#include "unit-distributer.h"
#include "worker-manager.h"
#include "scout-manager.h"
#include "micro-task-impl.h"

#include "informations/informations.h"

void Micro::clear()
{
	InformationKeeper::instance().clear();
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
	WorkerManager::instance().updateBases();
}

void Micro::tick()
{
	InformationKeeper::instance().tick();
	UnitDistributer::instance().tick();
	WorkerManager::instance().tick();
	ScoutManager::instance().tick();
}



