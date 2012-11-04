#include "zerg-strategie.hpp"
#include "resources.hpp"
#include "unit-morpher.hpp"
#include "unit-builder.hpp"
#include "mineral-line.hpp"
#include "supply.hpp"
#include "idle-unit-container.hpp"
#include "requirements.hpp"
#include "precondition-helper.hpp"
#include "building-placer.hpp"
#include "log.hpp"
#include <sstream>

using namespace BWAPI;
using namespace UnitTypes;

namespace {
	Precondition* waittill = NULL;
}

void ZergStrategieCode::onMatchBegin()
{
	if (Broodwar->self()->getRace() != Races::Zerg)
		return;
	
	setSupplyMode(Races::Zerg, SupplyMode::Auto);
	setRequirementsMode(RequirementsMode::Auto);

	for (int k=0; k<5; ++k)
		morphWorker(Zerg_Drone);
	morphUnitEx(Zerg_Overlord);
	buildUnitEx(Zerg_Spawning_Pool);
	
	for (int k=0; k<5; ++k)
		morphWorker(Zerg_Drone);
	buildRefinery(Zerg_Extractor);
	
    BuildingPositionPrecondition* pos = getNextExpo(Zerg_Hatchery);
	UnitPrecondition* hatch = buildUnit(pos, Zerg_Hatchery).first;
	waittill = hatch;
	
	UnitPrecondition* colony = buildUnit(Zerg_Creep_Colony).first;
	rememberIdle(morphUnit(colony, Zerg_Sunken_Colony));
	
	for (int k=0; k<5; ++k)
		morphWorker(Zerg_Drone);
	
	for (int k=0; k<10; ++k)
		morphUnitEx(Zerg_Zergling);
}

void ZergStrategieCode::onTick()
{
	if (Broodwar->self()->getRace() != Races::Zerg)
		return;

	if ((waittill == NULL) || (waittill->time == 0)) {
		release(waittill);
		
		for (int k=0; k<9; ++k)
			morphUnitEx(Zerg_Zergling);
            
        for (int k=0; k<3; ++k)
            morphUnitEx(Zerg_Mutalisk);
		
		waittill = morphUnit(Zerg_Zergling);
	}
}

void ZergStrategieCode::onMatchEnd()
{
	release(waittill);
}
