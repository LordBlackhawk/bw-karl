#include "zerg-strategie.hpp"
#include "resources.hpp"
#include "unit-morpher.hpp"
#include "unit-builder.hpp"
#include "mineral-line.hpp"
#include "idle-unit-container.hpp"
#include "precondition-helper.hpp"
#include "utils/debug.h"
#include <sstream>

using namespace BWAPI;

namespace {
	Precondition* waittill = NULL;
}

void ZergStrategieCode::onMatchBegin()
{
	if (Broodwar->self()->getRace() != Races::Zerg)
		return;

	for (int k=0; k<5; ++k)
		useWorker(morphUnit(UnitTypes::Zerg_Drone));
	
	UnitPrecondition* pool = buildUnit(UnitTypes::Zerg_Spawning_Pool).first;
	UnitPrecondition* colony = buildUnit(UnitTypes::Zerg_Creep_Colony).first;
	rememberIdle(morphUnit(colony, UnitTypes::Zerg_Sunken_Colony));
	
	for (int k=0; k<5; ++k)
		useWorker(morphUnit(BWAPI::UnitTypes::Zerg_Drone));
	
	for (int k=0; k<10; ++k)
		rememberIdle(morphUnit(UnitTypes::Zerg_Zergling));
	
	UnitPrecondition* hatch = buildUnit(UnitTypes::Zerg_Hatchery, pool).first;
	waittill = hatch;
}

void ZergStrategieCode::onTick()
{
	if (Broodwar->self()->getRace() != Races::Zerg)
		return;

	if ((waittill == NULL) || (waittill->time == 0)) {
		release(waittill);
		
		for (int k=0; k<9; ++k)
			rememberIdle(morphUnit(UnitTypes::Zerg_Zergling));
		
		waittill = morphUnit(UnitTypes::Zerg_Zergling);
	}
}

void ZergStrategieCode::onMatchEnd()
{
	release(waittill);
}
