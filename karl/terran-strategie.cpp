#include "terran-strategie.hpp"
#include "resources.hpp"
#include "unit-morpher.hpp"
#include "unit-builder.hpp"
#include "unit-trainer.hpp"
#include "mineral-line.hpp"
#include "supply.hpp"
#include "idle-unit-container.hpp"
#include "requirements.hpp"
#include "precondition-helper.hpp"
#include "utils/debug.h"
#include <sstream>

using namespace BWAPI;

namespace {
	UnitPrecondition* waittill = NULL;
}

void TerranStrategieCode::onMatchBegin()
{
	if (Broodwar->self()->getRace() != Races::Terran)
		return;
	
	setSupplyMode(Races::Terran, SupplyMode::Auto);
	setRequirementsMode(RequirementsMode::Auto);

	LOG << "Standard terran opening...";
	for (int k=0; k<10; ++k)
		trainWorker(UnitTypes::Terran_SCV);

	buildUnitEx(UnitTypes::Terran_Barracks);
	buildRefinery(UnitTypes::Terran_Refinery);
	
	for (int k=0; k<10; ++k)
		trainUnitEx(UnitTypes::Terran_Marine);
		
	//buildUnitEx(UnitTypes::Terran_Academy);

	//waittill = rememberSecond(trainUnit(UnitTypes::Terran_SCV));
}

void TerranStrategieCode::onTick()
{
	if (Broodwar->self()->getRace() != Races::Terran)
		return;

	/*
	if ((waittill == NULL) || (waittill->time == 0)) {
		rememberIdle(waittill);
		
		useWorker(rememberFirst(buildUnit(UnitTypes::Terran_Barracks)));
		
		for (int k=0; k<10; ++k)
			rememberIdle(rememberSecond(trainUnit(UnitTypes::Terran_Marine)));
		
		waittill = rememberSecond(trainUnit(UnitTypes::Terran_SCV));
	}
	*/
}

void TerranStrategieCode::onMatchEnd()
{
	release(waittill);
}
