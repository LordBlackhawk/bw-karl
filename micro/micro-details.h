#pragma once

#include "newplan/checkpoints.h"
#include "micro-task.h"

struct BuildBuildingDetails
{
	BWAPI::UnitType		ut;
	BWAPI::TilePosition	pos;
	BWAPI::Unit*		builder;
	BWAPI::Unit*		building;

	MicroTask			task;

	BuildBuildingDetails() : ut(BWAPI::UnitTypes::None), pos(BWAPI::TilePositions::None), builder(NULL), building(NULL)
	{ }
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
