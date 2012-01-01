#pragma once

#include "micro-task.h"
#include "building-placer.h"

#include "utils/debug.h"
#include "newplan/checkpoints.h"

struct BuildBuildingDetails
{
	BWAPI::UnitType		ut;
	BWAPI::TilePosition	pos;
	BWAPI::Unit*		builder;
	BWAPI::Unit*		building;

	MicroTask			task;
	bool 				reserved;

	BuildBuildingDetails() : ut(BWAPI::UnitTypes::None), pos(BWAPI::TilePositions::None), builder(NULL), building(NULL)
	{ }
	
	~BuildBuildingDetails()
	{
		if (reserved) {
			LOG1 << "Freeing tiles for " << ut.getName() << " on " << pos.x() << ", " << pos.y();
			BuildingPlacer::instance().freeTiles(ut, pos);
		}
	}
	
	void reserve()
	{
		LOG1 << "Reserving tiles for " << ut.getName() << " on " << pos.x() << ", " << pos.y();
		BuildingPlacer::instance().reserveTiles(ut, pos);
	}
};

struct BuildUnitDetails
{
	BWAPI::UnitType		ut;
	BWAPI::Unit*		builder;
	BWAPI::Unit*		result;

	MicroTask			task;
	
	BuildUnitDetails() : ut(BWAPI::UnitTypes::None), builder(NULL), result(NULL)
	{ }
};

struct TechDetails
{
	BWAPI::TechType		tt;
	BWAPI::Unit*		researcher;

	MicroTask			task;

	TechDetails() : tt(BWAPI::TechTypes::None), researcher(NULL)
	{ }
};

struct UpgradeDetails
{
	BWAPI::UpgradeType	gt;
	BWAPI::Unit*		upgrader;

	MicroTask			task;

	UpgradeDetails() : gt(BWAPI::UpgradeTypes::None), upgrader(NULL)
	{ }
};
