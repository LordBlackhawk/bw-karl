#pragma once

#include "bwplan/bwplan.h"
#include "utils/singleton.h"
#include <BWAPI.h>

struct WorkerDetails
{
	BWAPI::Unit*		builder;
	BWAPI::UnitType		type;
	BWAPI::TilePosition	location;
	BWAPI::Unit*		result;

	WorkerDetails()
	: builder(NULL), type(BWAPI::UnitTypes::Invalid), location(BWAPI::TilePositions::Invalid), result(NULL)
	{ }
};

struct WorkerGlobal
{
	void prepare(WorkerDetails*& details)
	{
		if (details == NULL)
			details = new WorkerDetails;
	}
};

DEF_CHECKPOINTCODE(CSendWorkerToBuildingPlace)
{
	WorkerGlobal& global = Singleton<WorkerGlobal>::instance();
	global.prepare(details);

	// TODO: Find location, find worker!

	if (status == OperationStatus::started) {
		details->builder->rightClick(details->location);
		return CheckPointResult::running;
	} else {
		// TODO: Check worker near location!
		if (!details->builder->isIdle())
			return CheckPointResult::running;
		return CheckPointResult::completed;
	}
}

DEF_CHECKPOINTCODE(CBuildBuilding)
{
	WorkerGlobal& global = Singleton<WorkerGlobal>::instance();
	global.prepare(details);

	if (details->builder == NULL)
		return CheckPointResult::failed;
	if (details->location == BWAPI::TilePositions::Invalid)
		return CheckPointResult::failed;
	if (details->type == BWAPI::UnitTypes::Invalid)
		details->type = getAssociation<OperationType, BWAPI::UnitType>();

	if (status == OperationStatus::started) {
		details->builder->build(details->location, details->type);
		return CheckPointResult::running;
	} else {
		result = NULL;
		for (auto it : Broodwar->self()->getUnits())
		{
			if (it.getTilePosition() != details->location)
				continue;
			if (it.getType() != details->type)
				continue;
			result = it;
		}
		if (result == NULL)
			return CheckPointResult::failed;
		return CheckPointResult::completed;
	}
}

DEF_CHECKPOINTCODE(CBuildingFinished)
{
	WorkerGlobal& global = Singleton<WorkerGlobal>::instance();
	global.prepare(details);

	if (details->result == NULL)
		return CheckPointResult::failed;

	BWAPI::Unit* result = details->result;
	if (result->isCompleted())
		return CheckPointResult::completed;

	return CheckPointResult::running;
}

DEF_CHECKPOINTCODE(CMorphUnit)
{
	WorkerGlobal& global = Singleton<WorkerGlobal>::instance();
	global.prepare(details);

	// TODO: Find Larva
	if (details->type == BWAPI::UnitTypes::Invalid)
		details->type = getAssociation<OperationType, BWAPI::UnitType>();

	if (status == OperationStatus::started) {
		details->builder->morph(details->type);
		return CheckPointResult::running;
	} else {
		if (!details->builder->isMorphing())
			return CheckPointResult::failed;
		details->result = details->builder;
		return CheckPointResult::finished;
	}
}

DEF_CHECKPOINTCODE(CTrainUnit)
{
	WorkerGlobal& global = Singleton<WorkerGlobal>::instance();
	global.prepare(details);

	// TODO: Find Trainer
	if (details->type == BWAPI::UnitTypes::Invalid)
		details->type = getAssociation<OperationType, BWAPI::UnitType>();

	if (status == OperationStatus::started) {
		details->builder->train(details->type);
		return CheckPointResult::running;
	} else {
		if (!details->builder->isTraining())
			return CheckPointResult::failed;
		result = NULL;
		for (auto it : Broodwar->self()->getUnits()) {
			// TODO: Near Trainer!
			if (it->getType() != details->type)
				continue;
			result = it;
		}
		if (result == NULL)
			return CheckPointResult::failed;
		return CheckPointResult::finished;
	}
}

DEF_CHECKPOINTCODE(CUnitFinished)
{
	WorkerGlobal& global = Singleton<WorkerGlobal>::instance();
	global.prepare(details);

	if (details->result == NULL)
		return CheckPointResult::failed;

	BWAPI::Unit* result = details->result;
	if (result->isCompleted())
		return CheckPointResult::completed;

	return CheckPointResult::running;
}