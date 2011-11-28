#pragma once

#include "micro-details.h"
#include "unit-finder.h"
#include "building-placer.h"
#include "worker-manager.h"
#include "task-constructor.h"
#include "bwapi-helper.h"
#include "newplan/operations.h"

#include "utils/debug.h"

#define ASSERT(eq) \
	if (!(eq)) { LOG << "Assertation(" << __func__ << ") failed, while handling " << op.getName() << "! " << #eq; return CheckPointResult::failed; }

namespace {
	CheckPointResult::type WaitForTask(MicroTask& task)
	{
		switch (task.tick())
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
	
	BWAPI::Unit* findBuilding(BWAPI::Unit* builder, const BWAPI::UnitType& ut, const BWAPI::TilePosition& pos)
	{
		if (ut.isRefinery()) {
			BWAPI::Unit* result = UnitFinder::instance().findGlobal(ut, pos);
			if (result != NULL)
				MicroTaskManager::instance().onUnitAdded(result);
			return result;
		} else if (ut.getRace() == BWAPI::Races::Zerg) {
			return (builder->getType() == ut) ? builder : NULL;
		} else {
			return UnitFinder::instance().find(ut, pos);
		}
	}
}

CheckPointResult::type CSendGasWorker(Operation& /*op*/)
{
	WorkerManager::instance().sendGasWorker();
	return CheckPointResult::completed;
}

CheckPointResult::type CReturnGasWorker(Operation& /*op*/)
{
	WorkerManager::instance().returnGasWorker();
	return CheckPointResult::completed;
}

CheckPointResult::type CSendWorkerToBuildingPlace(Operation& op)
{
	boost::shared_ptr<BuildBuildingDetails> details = op.getDetails<BuildBuildingDetails>(); // Makes initialisation!!!
	ASSERT(details.use_count() > 0);

	if (op.status() == OperationStatus::started) {

		if (details->ut == BWAPI::UnitTypes::None)
			details->ut = op.associatedUnitType();
			
		BWAPI::UnitType but = details->ut.whatBuilds().first;
		if (but.isBuilding())
			details->ut = but;
	
		if (details->pos == BWAPI::TilePositions::None)
			details->pos = BuildingPlacer::instance().find(details->ut);
	
		if (details->builder == NULL)
			details->builder = UnitFinder::instance().findWorker(details->ut.getRace(), details->pos);

		ASSERT(details->ut != BWAPI::UnitTypes::None);
		ASSERT(details->pos != BWAPI::TilePositions::None);
		ASSERT(details->builder != NULL);

		details->task = createLongMove(getBuildingCenter(details->pos, details->ut));
		MicroTaskManager::instance().pushTask(details->builder, details->task);

	}

	drawBuildingPosition(details->ut, details->pos);
	return WaitForTask(details->task);
}

CheckPointResult::type CBuildBuilding(Operation& op)
{
	boost::shared_ptr<BuildBuildingDetails> details = op.getDetails<BuildBuildingDetails>(); // Makes initialisation!!!
	ASSERT(details.use_count() > 0);

	if (op.status() == OperationStatus::started) {		
		
		if (details->building != NULL) {
			LOG1 << "Recognized building in 2 steps.";
			details->builder = details->building;
			details->ut      = op.associatedUnitType();
		}
		if (details->ut == BWAPI::UnitTypes::None)
			details->ut = op.associatedUnitType();
		
		if (details->builder == NULL)
			details->builder = UnitFinder::instance().findIdle(details->ut.whatBuilds().first);
		ASSERT(details->ut != BWAPI::UnitTypes::None);
		ASSERT(details->builder != NULL);
		
		if (details->pos == BWAPI::TilePositions::None)
			details->pos = details->builder->getTilePosition();
		ASSERT(details->pos != BWAPI::TilePositions::None);
		
		LOG1 << "Calling BuildBuilding(" << details->ut.getName() << ")...";
		
		if (!details->builder->getType().isWorker()) {
			// morph-Building:
			details->task    = createMorph(details->ut);
		} else {
			// build-Building:
			details->task    = createBuild(details->ut, details->pos);
		}
		MicroTaskManager::instance().pushTask(details->builder, details->task);

	}

	op.rescheduleBegin(BWAPI::Broodwar->getFrameCount()+1);
	drawBuildingPosition(details->ut, details->pos);

	CheckPointResult::type result = WaitForTask(details->task);
	if (result == CheckPointResult::completed) {
		details->building = findBuilding(details->builder, details->ut, details->pos);
		ASSERT(details->building != NULL);
		LOG1 << "BuildBuilding(" << details->ut.getName() << ") completed.";
	}
	return result;
}

CheckPointResult::type CBuildingFinished(Operation& op)
{
	boost::shared_ptr<BuildBuildingDetails> details = op.getDetails<BuildBuildingDetails>(); // Makes initialisation!!!
	ASSERT(details.use_count() > 0);

	if (op.status() == OperationStatus::started) {

		LOG1 << "Calling BuildFinished(" << details->ut.getName() << ")...";
		ASSERT(details->building != NULL);

		details->task = createBuildObserver();
		MicroTaskManager::instance().pushTask(details->building, details->task);
		
		op.rescheduleEnd(BWAPI::Broodwar->getFrameCount() + details->building->getRemainingBuildTime());

	}

	CheckPointResult::type result = WaitForTask(details->task);
	if (result == CheckPointResult::completed)
		LOG1 << "BuildFinished(" << details->ut.getName() << ") completed.";
	return result;
}

CheckPointResult::type CBuildAddon(Operation& /*op*/)
{
	return CheckPointResult::failed;
}

CheckPointResult::type CBuildAddonFinished(Operation& /*op*/)
{
	return CheckPointResult::failed;
}

CheckPointResult::type CMorphUnit(Operation& op)
{
	boost::shared_ptr<BuildUnitDetails> details = op.getDetails<BuildUnitDetails>(); // Makes initialisation!!!
	ASSERT(details.use_count() > 0);

	if (op.status() == OperationStatus::started) {
	
		if (details->result != NULL) {
			LOG1 << "Recognized morphing in 2 steps.";
			details->builder = details->result;
			details->ut      = op.associatedUnitType();
		} else {
			if (details->ut == BWAPI::UnitTypes::None)
				details->ut = op.associatedUnitType();
				
			if (details->ut.whatBuilds().first != BWAPI::UnitTypes::Zerg_Larva) {
				LOG1 << "Morphing requirements first...";
				details->ut = details->ut.whatBuilds().first;
			}
		
			if (details->builder == NULL)
				details->builder = UnitFinder::instance().findIdle(details->ut.whatBuilds().first);
		}

		ASSERT(details->ut != BWAPI::UnitTypes::None);
		ASSERT(details->builder != NULL);

		details->result = details->builder;
		details->task = createMorph(details->ut);
		MicroTaskManager::instance().pushTask(details->builder, details->task);
		
		LOG3 << "Started morphing " << details->ut.getName() << "...";

	}
	
	op.rescheduleBegin(BWAPI::Broodwar->getFrameCount()+1);
	return WaitForTask(details->task);
}

CheckPointResult::type CCombineUnit(Operation& /*op*/)
{
	return CheckPointResult::failed;
}

CheckPointResult::type CTrainUnit(Operation& op)
{
	boost::shared_ptr<BuildUnitDetails> details = op.getDetails<BuildUnitDetails>(); // Makes initialisation!!!
	ASSERT(details.use_count() > 0);

	if (op.status() == OperationStatus::started) {

		if (details->ut == BWAPI::UnitTypes::None)
			details->ut = op.associatedUnitType();

		if (details->builder == NULL)
			details->builder = UnitFinder::instance().findIdle(details->ut.whatBuilds().first);

		ASSERT(details->ut != BWAPI::UnitTypes::None);
		ASSERT(details->builder != NULL);

		details->task = createTrain(details->ut);
		MicroTaskManager::instance().pushTask(details->builder, details->task);

	}

	CheckPointResult::type result = WaitForTask(details->task);
	if (result == CheckPointResult::completed) {
		details->result = UnitFinder::instance().find(details->ut, details->builder->getPosition());
		if (details->result == NULL)
			return CheckPointResult::failed;
	}
	return result;
}

CheckPointResult::type CUnitFinished(Operation& op)
{
	boost::shared_ptr<BuildUnitDetails> details = op.getDetails<BuildUnitDetails>(); // Makes initialisation!!!
	ASSERT(details.use_count() > 0);

	if (op.status() == OperationStatus::started) {

		ASSERT(details->result != NULL);

		if (details->result->isMorphing()) {
			details->task = createMorphObserver();
		} else if (details->result->isBeingConstructed()) {
			details->task = createTrainObserver();
		} else {
			return CheckPointResult::failed;
		}
		MicroTaskManager::instance().pushTask(details->result, details->task);
		
		op.rescheduleEnd(BWAPI::Broodwar->getFrameCount() + details->result->getRemainingBuildTime());
	
	}

	return WaitForTask(details->task);
}

CheckPointResult::type CTechStart(Operation& op)
{
	boost::shared_ptr<TechDetails> details = op.getDetails<TechDetails>(); // Makes initialisation!!!
	ASSERT(details.use_count() > 0);

	if (op.status() == OperationStatus::started) {

		if (details->tt == BWAPI::TechTypes::None)
			details->tt = op.associatedTechType();

		if (details->researcher == NULL)
			details->researcher = UnitFinder::instance().findIdle(details->tt.whatResearches());

		ASSERT(details->tt != BWAPI::TechTypes::None);
		ASSERT(details->researcher != NULL);

		details->task = createTech(details->tt);
		MicroTaskManager::instance().pushTask(details->researcher, details->task);

	}

	return WaitForTask(details->task);
}

CheckPointResult::type CTechFinished(Operation& op)
{
	boost::shared_ptr<TechDetails> details = op.getDetails<TechDetails>(); // Makes initialisation!!!
	ASSERT(details.use_count() > 0);

	if (op.status() == OperationStatus::started) {

		ASSERT(details->researcher != NULL);

		details->task = createTechObserver();
		MicroTaskManager::instance().pushTask(details->researcher, details->task);

	}

	return WaitForTask(details->task);
}

CheckPointResult::type CUpgradeStart(Operation& op)
{
	boost::shared_ptr<UpgradeDetails> details = op.getDetails<UpgradeDetails>(); // Makes initialisation!!!
	ASSERT(details.use_count() > 0);

	if (op.status() == OperationStatus::started) {

		if (details->gt == BWAPI::UpgradeTypes::None)
			details->gt = op.associatedUpgradeType();
		ASSERT(details->gt != BWAPI::UpgradeTypes::None);
		
		if (details->upgrader == NULL)
			details->upgrader = UnitFinder::instance().findIdle(details->gt.whatUpgrades());
		ASSERT(details->upgrader != NULL);

		details->task = createUpgrade(details->gt);
		MicroTaskManager::instance().pushTask(details->upgrader, details->task);

	}

	return WaitForTask(details->task);
}

CheckPointResult::type CUpgradeFinished(Operation& op)
{
	boost::shared_ptr<UpgradeDetails> details = op.getDetails<UpgradeDetails>(); // Makes initialisation!!!
	ASSERT(details.use_count() > 0);

	if (op.status() == OperationStatus::started) {

		ASSERT(details->upgrader != NULL);

		details->task = createUpgradeObserver();
		MicroTaskManager::instance().pushTask(details->upgrader, details->task);

	}

	return WaitForTask(details->task);
}

#undef ASSERT
