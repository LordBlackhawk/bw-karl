#pragma once

#include "informations/informations.h"
#include "building-placer.h"

#include "utils/debug.h"
#include "newplan/checkpoints.h"

struct BuildBuildingDetails
{
	BWAPI::UnitType		ut;
	BWAPI::TilePosition	pos;
	UnitInfoPtr			builder;
	UnitInfoPtr			building;

	MicroTaskPtr		task;
	bool 				reserved;

	BuildBuildingDetails() : ut(BWAPI::UnitTypes::None), pos(BWAPI::TilePositions::None)
	{ }
	
	~BuildBuildingDetails()
	{
		if (reserved) {
			LOG2 << "Freeing tiles for " << ut.getName() << " on " << pos.x() << ", " << pos.y();
			BuildingPlacer::instance().freeTiles(ut, pos);
		}
	}
	
	void reserve()
	{
		LOG2 << "Reserving tiles for " << ut.getName() << " on " << pos.x() << ", " << pos.y();
		BuildingPlacer::instance().reserveTiles(ut, pos);
	}
};

struct BuildUnitDetails
{
	BWAPI::UnitType		ut;
	UnitInfoPtr			builder;
	UnitInfoPtr			result;

	MicroTaskPtr		task;
	
	BuildUnitDetails() : ut(BWAPI::UnitTypes::None)
	{ }
};

struct TechDetails
{
	BWAPI::TechType		tt;
	UnitInfoPtr			researcher;

	MicroTaskPtr		task;

	TechDetails() : tt(BWAPI::TechTypes::None)
	{ }
};

struct UpgradeDetails
{
	BWAPI::UpgradeType	gt;
	UnitInfoPtr			upgrader;

	MicroTaskPtr		task;

	UpgradeDetails() : gt(BWAPI::UpgradeTypes::None)
	{ }
};
