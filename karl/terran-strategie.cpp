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
	bool academystarted = false;
}

void TerranStrategieCode::onMatchBegin()
{
	if (Broodwar->self()->getRace() != Races::Terran)
		return;
	
	setSupplyMode(Races::Terran, SupplyMode::Auto);
	setRequirementsMode(RequirementsMode::Auto);

	LOG << "Standard terran opening...";
	for (int k=0; k<4; ++k)
		trainWorker(UnitTypes::Terran_SCV);
	
	buildUnitEx(UnitTypes::Terran_Supply_Depot);
	
	for (int k=0; k<10; ++k)
		trainWorker(UnitTypes::Terran_SCV);

	buildUnitEx(UnitTypes::Terran_Barracks);
	
	for (int k=0; k<6; ++k)
		trainUnitEx(UnitTypes::Terran_Marine);
}

void TerranStrategieCode::onTick()
{
	if (Broodwar->self()->getRace() != Races::Terran)
		return;
		
	int now = Broodwar->getFrameCount();
	if (now % 10 != 7)
		return;
	
	if (nextUnitAvaiable(UnitTypes::Terran_Barracks) < now + 100) {
		if (!academystarted) {
			buildUnitEx(UnitTypes::Terran_Academy);
			buildRefinery(UnitTypes::Terran_Refinery);
			academystarted = true;
		}
			
		for (auto k=0; k<5; ++k)
			trainUnitEx(UnitTypes::Terran_Marine);
		trainUnitEx(UnitTypes::Terran_Medic);
		trainUnitEx(UnitTypes::Terran_Firebat);
	} else if ((Broodwar->self()->minerals() > 400) && (buildUnitPlanSize(UnitTypes::Terran_Barracks) < 1)) {
		buildUnitEx(UnitTypes::Terran_Barracks);
		for (int k=0; k<3; ++k)
			trainWorker(UnitTypes::Terran_SCV);
	}
}

void TerranStrategieCode::onMatchEnd()
{
	release(waittill);
}
