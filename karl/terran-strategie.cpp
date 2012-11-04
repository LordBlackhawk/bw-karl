#include "terran-strategie.hpp"
#include "resources.hpp"
#include "unit-morpher.hpp"
#include "unit-builder.hpp"
#include "unit-trainer.hpp"
#include "mineral-line.hpp"
#include "supply.hpp"
#include "idle-unit-container.hpp"
#include "requirements.hpp"
#include "tech-upgrades.hpp"
#include "precondition-helper.hpp"
#include "log.hpp"
#include <sstream>

using namespace BWAPI;
using namespace BWAPI::UnitTypes;

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
		trainWorker(Terran_SCV);
	
	buildUnitEx(Terran_Supply_Depot);
	
	for (int k=0; k<10; ++k)
		trainWorker(Terran_SCV);

	buildUnitEx(Terran_Barracks);
	
	for (int k=0; k<6; ++k)
		trainUnitEx(Terran_Marine);
}

void TerranStrategieCode::onTick()
{
	if (Broodwar->self()->getRace() != Races::Terran)
		return;
		
	int now = Broodwar->getFrameCount();
	if (now % 10 != 7)
		return;
	
	if (nextUnitAvaiable(Terran_Barracks) < now + 500) {
		if (!academystarted) {
			buildUnitEx(Terran_Academy);
			buildRefinery(Terran_Refinery);
			researchTechEx(TechTypes::Stim_Packs);
			upgradeTechEx(UpgradeTypes::U_238_Shells);
			academystarted = true;
		}
			
		for (auto k=0; k<4; ++k)
			trainUnitEx(Terran_Marine);
		trainUnitEx(Terran_Medic);
		trainUnitEx(Terran_Firebat);
	} else if ((Broodwar->self()->minerals() > 400) && (buildUnitPlanSize(Terran_Barracks) < 1)) {
		buildUnitEx(Terran_Barracks);
		for (int k=0; k<3; ++k)
			trainWorker(Terran_SCV);
	}
}

void TerranStrategieCode::onMatchEnd()
{
	release(waittill);
}
