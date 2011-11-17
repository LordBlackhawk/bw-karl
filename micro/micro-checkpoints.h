#pragma once

#include "micro-details.h"
#include "unit-finder.h"
#include "building-placer.h"
#include "worker-manager.h"
#include "task-constructor.h"

namespace {
	CheckPointResult::type WaitForTask(MicroTask& task)
	{
		switch (task.status())
		{
			case TaskStatus::failed:
				task = MicroTask();
				return CheckPointResult::failed;
			case TaskStatus::completed:
				task = MicroTask();
				return CheckPointResult::completed;
			default:
				return CheckPointResult::running;
		}
	}
}

CheckPointResult::type CSendGasWorker(Operation& op)
{
	WorkerManager::instance().sendGasWorker();
	return CheckPointResult::completed;
}

CheckPointResult::type CReturnGasWorker(Operation& op)
{
	WorkerManager::instance().returnGasWorker();
	return CheckPointResult::completed;
}

CheckPointResult::type CSendWorkerToBuildingPlace(Operation& op)
{
	boost::shared_ptr<BuildBuildingDetails> details = op.getDetails<BuildBuildingDetails>(); // Makes initialisation!!!

	if (op.status() == OperationStatus::started) {

		if (details->ut == BWAPI::UnitTypes::None)
			details->ut = op.associatedUnitType();
	
		if (details->pos == BWAPI::TilePositions::None)
			details->pos = BuildingPlacer::instance().find(details->ut);
	
		if (details->builder == NULL)
			detail->builder = UnitFinder::instance().findWorker(details->ut.getRace(), details->pos);

		assert(details->ut != BWAPI::UnitTypes::None);
		assert(details->pos != BWAPI::TilePositions::None);
		assert(details->builder != NULL);

		details->task = createLongMove(details->pos);
		MicroTaskManager::instance().pushTask(details->builder, details->task);

		return CheckPointResult::running;

	} else {

		return WaitForTask(details->task);

	}
}

CheckPointResult::type CBuildBuilding(Operation& op)
{
	boost::shared_ptr<BuildBuildingDetails> details = op.getDetails<BuildBuildingDetails>(); // Makes initialisation!!!

	if (op.status() == OperationStatus::started) {

		assert(details->ut != BWAPI::UnitTypes::None);
		assert(details->pos != BWAPI::TilePositions::None);
		assert(details->builder != NULL);

		details->task = createBuild(details->ut, details->pos);
		MicroTaskManager::instance().pushTask(details->builder, details->task);

		return CheckPointResult::running;

	} else {

		CheckPointResult::type result = WaitForTask(details->task);
		if (result == CheckPointResult::completed) {
			details->building = UnitFinder::instance().find(details->ut, details->pos);
			if (details->building == NULL)
				return CheckPointResult::failed;
		}
		return result;

	}
}

CheckPointResult::type CBuildingFinished(Operation& op)
{
	boost::shared_ptr<BuildBuildingDetails> details = op.getDetails<BuildBuildingDetails>(); // Makes initialisation!!!

	if (op.status() == OperationStatus::started) {

		assert(details->building != NULL);

		details->task = createBuildObserver();
		MicroTaskManager::instance().pushTask(details->building, details->task);

		return CheckPointResult::running;

	} else {

		return WaitForTask(details->task);

	}
}

CheckPointResult::type CBuildAddon(Operation& op)
{
	return CheckPointResult::failed;
}

CheckPointResult::type CBuildAddonFinished(Operation& op)
{
	return CheckPointResult::failed;
}

CheckPointResult::type CMorphUnit(Operation& op)
{
	boost::shared_ptr<BuildBuildingDetails> details = op.getDetails<BuildBuildingDetails>(); // Makes initialisation!!!

	if (op.status() == OperationStatus::started) {

		if (details->ut == BWAPI::UnitTypes::None)
			details->ut = op.associatedUnitType();

		if (details->builder == NULL)
			details->builder = UnitFinder::instance().findIdle(details->ut.whatBuilds());

		assert(details->ut != BWAPI::UnitTypes::None);
		assert(details->builder != NULL);

		details->result = details->builder;
		details->task = createMorph(details->ut);
		MicroTaskManager::instance().pushTask(details->builder, details->task);

		return CheckPointResult::running;

	} else {

		return WaitForTask(details->task);

	}
}

CheckPointResult::type CCombineUnit(Operation& op)
{
	return CheckPointResult::failed;
}

CheckPointResult::type CTrainUnit(Operation& op)
{
	boost::shared_ptr<BuildBuildingDetails> details = op.getDetails<BuildBuildingDetails>(); // Makes initialisation!!!

	if (op.status() == OperationStatus::started) {

		if (details->ut == BWAPI::UnitTypes::None)
			details->ut = op.associatedUnitType();

		if (details->builder == NULL)
			details->builder = UnitFinder::instance().findIdle(details->ut.whatBuilds());

		assert(details->ut != BWAPI::UnitTypes::None);
		assert(details->builder != NULL);

		details->task = createTrain(details->ut);
		MicroTaskManager::instance().pushTask(details->builder, details->task);

		return CheckPointResult::running;

	} else {

		CheckPointResult::type result = WaitForTask(details->task);
		if (result == CheckPointResult::completed) {
			details->result = UnitFinder::instance().find(details->ut, details->builder->getPosition());
			if (details->result == NULL)
				return CheckPointResult::failed;
		}
		return result;

	}
}

CheckPointResult::type CUnitFinished(Operation& op)
{
	boost::shared_ptr<BuildBuildingDetails> details = op.getDetails<BuildBuildingDetails>(); // Makes initialisation!!!

	if (op.status() == OperationStatus::started) {

		assert(details->result != NULL);

		if (details->result->isMorphing()) {
			details->task = createMorphObserver();
		} else if (details->result->isBeingConstructed()) {
			details->task = createTrainObserver();
		} else {
			return CheckPointResult::failed;
		}
		MicroTaskManager::instance().pushTask(details->result, details->task);

		return CheckPointResult::running;

	} else {

		return WaitForTask(details->task);

	}
}

CheckPointResult::type CTechStart(Operation& op)
{
	boost::shared_ptr<TechDetails> details = op.getDetails<TechDetails>(); // Makes initialisation!!!

	if (op.status() == OperationStatus::started) {

		if (details->tt == BWAPI::TechTypes::None)
			details->tt = op.associatedTechType();

		if (details->researcher == NULL)
			details->researcher = UnitFinder::findIdle(details->tt.whatResearches());

		assert(details->tt != BWAPI::TechTypes::None);
		assert(details->researcher != NULL);

		details->task = createTech(details->tt);
		MicroTaskManager::instance().pushTask(details->researcher, details->task);

		return CheckPointResult::running;

	} else {

		return WaitForTask(details->task);

	}
}

CheckPointResult::type CTechFinished(Operation& op)
{
	boost::shared_ptr<TechDetails> details = op.getDetails<TechDetails>(); // Makes initialisation!!!

	if (op.status() == OperationStatus::started) {

		assert(details->researcher != NULL);

		details->task = createTechObserver();
		MicroTaskManager::instance().pushTask(details->researcher, details->task);

		return CheckPointResult::running;

	} else {

		return WaitForTask(details->task);

	}
}

CheckPointResult::type CUpgradeStart(Operation& op)
{
	boost::shared_ptr<UpgradeDetails> details = op.getDetails<UpgradeDetails>(); // Makes initialisation!!!

	if (op.status() == OperationStatus::started) {

		if (details->tt == BWAPI::TechTypes::None)
			details->tt = op.associatedTechType();

		if (details->upgrader == NULL)
			details->upgrader = UnitFinder::findIdle(details->tt.whatResearches());

		assert(details->tt != BWAPI::TechTypes::None);
		assert(details->upgrader != NULL);

		details->task = createUpgrade(details->tt);
		MicroTaskManager::instance().pushTask(details->upgrader, details->task);

		return CheckPointResult::running;

	} else {

		return WaitForTask(details->task);

	}
}

CheckPointResult::type CUpgradeFinished(Operation& op)
{
	boost::shared_ptr<UpgradeDetails> details = op.getDetails<UpgradeDetails>(); // Makes initialisation!!!

	if (op.status() == OperationStatus::started) {

		assert(details->upgrader != NULL);

		details->task = createUpgradeObserver();
		MicroTaskManager::instance().pushTask(details->upgrader, details->task);

		return CheckPointResult::running;

	} else {

		return WaitForTask(details->task);

	}
}

