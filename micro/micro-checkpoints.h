#pragma once

#include "micro-details.h"
#include "unit-finder.h"
#include "building-placer.h"
#include "worker-manager.h"
#include "task-constructor.h"
#include "newplan/operations.h"

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
	
	void drawBuildingPosition(const BWAPI::UnitType& ut, const BWAPI::TilePosition& tp)
	{
		BWAPI::Broodwar->drawBoxMap(tp.x()*32, tp.y()*32, tp.x()*32+ut.tileWidth()*32, tp.y()*32+ut.tileHeight()*32, BWAPI::Colors::Green);
		BWAPI::Broodwar->drawTextMap(tp.x()*32, tp.y()*32, "%s", ut.getName().c_str());
	}
	
	BWAPI::Position getBuildingCenter(const BWAPI::TilePosition& pos, const BWAPI::UnitType& ut)
	{
		BWAPI::Position result = BWAPI::Position(pos);
		result += BWAPI::Position(ut.tileWidth() * 16, ut.tileHeight() * 16);
		return result;
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
	assert(details.use_count() > 0);

	if (op.status() == OperationStatus::started) {

		if (details->ut == BWAPI::UnitTypes::None)
			details->ut = op.associatedUnitType();
	
		if (details->pos == BWAPI::TilePositions::None)
			details->pos = BuildingPlacer::instance().find(details->ut);
	
		if (details->builder == NULL)
			details->builder = UnitFinder::instance().findWorker(details->ut.getRace(), details->pos);

		assert(details->ut != BWAPI::UnitTypes::None);
		assert(details->pos != BWAPI::TilePositions::None);
		assert(details->builder != NULL);

		details->task = createLongMove(getBuildingCenter(details->pos, details->ut));
		MicroTaskManager::instance().pushTask(details->builder, details->task);

	}

	drawBuildingPosition(details->ut, details->pos);
	return WaitForTask(details->task);
}

CheckPointResult::type CBuildBuilding(Operation& op)
{
	boost::shared_ptr<BuildBuildingDetails> details = op.getDetails<BuildBuildingDetails>(); // Makes initialisation!!!
	assert(details.use_count() > 0);

	if (op.status() == OperationStatus::started) {

		assert(details->ut != BWAPI::UnitTypes::None);
		assert(details->pos != BWAPI::TilePositions::None);
		assert(details->builder != NULL);
	
		std::clog << BWAPI::Broodwar->getFrameCount() << ": Calling BuildBuilding(" << details->ut.getName() << ")...\n";

		details->task = createBuild(details->ut, details->pos);
		MicroTaskManager::instance().pushTask(details->builder, details->task);

	}

	op.rescheduleBegin(BWAPI::Broodwar->getFrameCount()+1);
	drawBuildingPosition(details->ut, details->pos);
	//std::clog << "Calling BuildBuilding (running)...\n";
	CheckPointResult::type result = WaitForTask(details->task);
	if (result == CheckPointResult::completed) {
		if ((details->ut.getRace() == BWAPI::Races::Zerg) && (details->ut != BWAPI::UnitTypes::Zerg_Extractor)) {
			if (details->builder->getType() == details->ut) {
				details->building = details->builder;
			} else {
				return CheckPointResult::failed;
			}
		} else {
			details->building = UnitFinder::instance().find(details->ut, details->pos);
			if (details->building == NULL) {
				std::clog << "could not find building " << details->ut.getName() << ", so operation fails.\n";
				return CheckPointResult::failed;
			}
		}
	}
	return result;
}

CheckPointResult::type CBuildingFinished(Operation& op)
{
	boost::shared_ptr<BuildBuildingDetails> details = op.getDetails<BuildBuildingDetails>(); // Makes initialisation!!!
	assert(details.use_count() > 0);

	if (op.status() == OperationStatus::started) {

		std::clog << "Calling BuildFinished(" << details->ut.getName() << ")...\n";
		assert(details->building != NULL);

		details->task = createBuildObserver();
		MicroTaskManager::instance().pushTask(details->building, details->task);
		
		op.rescheduleEnd(BWAPI::Broodwar->getFrameCount() + details->building->getRemainingBuildTime());

	}

	//drawBuildingPosition(details->ut, details->pos);
	//std::clog << "Calling BuildFinished (running)...\n";
	return WaitForTask(details->task);
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
	assert(details.use_count() > 0);

	if (op.status() == OperationStatus::started) {

		if (details->ut == BWAPI::UnitTypes::None)
			details->ut = op.associatedUnitType();

		if (details->builder == NULL)
			details->builder = UnitFinder::instance().findIdle(details->ut.whatBuilds().first);

		assert(details->ut != BWAPI::UnitTypes::None);
		assert(details->builder != NULL);

		details->result = details->builder;
		details->task = createMorph(details->ut);
		MicroTaskManager::instance().pushTask(details->builder, details->task);
		
		std::clog << BWAPI::Broodwar->getFrameCount() << ": Started morphing " << details->ut.getName() << "...\n";

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
	assert(details.use_count() > 0);

	if (op.status() == OperationStatus::started) {

		if (details->ut == BWAPI::UnitTypes::None)
			details->ut = op.associatedUnitType();

		if (details->builder == NULL)
			details->builder = UnitFinder::instance().findIdle(details->ut.whatBuilds().first);

		assert(details->ut != BWAPI::UnitTypes::None);
		assert(details->builder != NULL);

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
	assert(details.use_count() > 0);

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
		
		op.rescheduleEnd(BWAPI::Broodwar->getFrameCount() + details->result->getRemainingBuildTime());
	
	}

	return WaitForTask(details->task);
}

CheckPointResult::type CTechStart(Operation& op)
{
	boost::shared_ptr<TechDetails> details = op.getDetails<TechDetails>(); // Makes initialisation!!!
	assert(details.use_count() > 0);

	if (op.status() == OperationStatus::started) {

		if (details->tt == BWAPI::TechTypes::None)
			details->tt = op.associatedTechType();

		if (details->researcher == NULL)
			details->researcher = UnitFinder::instance().findIdle(details->tt.whatResearches());

		assert(details->tt != BWAPI::TechTypes::None);
		assert(details->researcher != NULL);

		details->task = createTech(details->tt);
		MicroTaskManager::instance().pushTask(details->researcher, details->task);

	}

	return WaitForTask(details->task);
}

CheckPointResult::type CTechFinished(Operation& op)
{
	boost::shared_ptr<TechDetails> details = op.getDetails<TechDetails>(); // Makes initialisation!!!
	assert(details.use_count() > 0);

	if (op.status() == OperationStatus::started) {

		assert(details->researcher != NULL);

		details->task = createTechObserver();
		MicroTaskManager::instance().pushTask(details->researcher, details->task);

	}

	return WaitForTask(details->task);
}

CheckPointResult::type CUpgradeStart(Operation& op)
{
	boost::shared_ptr<UpgradeDetails> details = op.getDetails<UpgradeDetails>(); // Makes initialisation!!!
	assert(details.use_count() > 0);

	if (op.status() == OperationStatus::started) {

		if (details->gt == BWAPI::TechTypes::None)
			details->gt = op.associatedUpgradeType();

		if (details->upgrader == NULL)
			details->upgrader = UnitFinder::instance().findIdle(details->gt.whatUpgrades());

		assert(details->gt != BWAPI::TechTypes::None);
		assert(details->upgrader != NULL);

		details->task = createUpgrade(details->gt);
		MicroTaskManager::instance().pushTask(details->upgrader, details->task);

	}

	return WaitForTask(details->task);
}

CheckPointResult::type CUpgradeFinished(Operation& op)
{
	boost::shared_ptr<UpgradeDetails> details = op.getDetails<UpgradeDetails>(); // Makes initialisation!!!
	assert(details.use_count() > 0);

	if (op.status() == OperationStatus::started) {

		assert(details->upgrader != NULL);

		details->task = createUpgradeObserver();
		MicroTaskManager::instance().pushTask(details->upgrader, details->task);

	}

	return WaitForTask(details->task);
}

