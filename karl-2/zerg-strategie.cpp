#include "zerg-strategie.hpp"
#include "resources.hpp"
#include "unit-morpher.hpp"
#include "unit-builder.hpp"
#include "mineral-line.hpp"
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

	for (int k=0; k<5; ++k) {
		UnitPrecondition* worker = morphUnit(BWAPI::UnitTypes::Zerg_Drone);
		if (worker == NULL) {
			LOG << "Error in morphUnit(): returns NULL.";
			continue;
		}
		useWorker(worker);
	}
	
	UnitPrecondition* pool = buildUnit(UnitTypes::Zerg_Spawning_Pool).first;
	if (pool == NULL) {
		LOG << "Error in buildUnit(Pool): returns NULL.";
		return;
	}

	UnitPrecondition* colony = buildUnit(UnitTypes::Zerg_Creep_Colony).first;
	if (colony == NULL) {
		LOG << "Error in buildUnit(CreepColony): returns NULL.";
		return;
	}
	
	if (morphUnit(colony, UnitTypes::Zerg_Sunken_Colony, pool) == NULL) {
		LOG << "Error in morph(Sunken): returns NULL.";
		return;
	}
	
	for (int k=0; k<5; ++k) {
		UnitPrecondition* worker = morphUnit(BWAPI::UnitTypes::Zerg_Drone);
		if (worker == NULL) {
			LOG << "Error in morphUnit(): returns NULL.";
			continue;
		}
		useWorker(worker);
	}
	
	for (int k=0; k<10; ++k)
		if (morphUnit(UnitTypes::Zerg_Zergling, pool) == NULL) {
			LOG << "Error in morph(Zergling): returns NULL.";
			return;
		}
	
	UnitPrecondition* hatch = buildUnit(UnitTypes::Zerg_Hatchery, pool).first;
	if (hatch == NULL) {
		LOG << "Error in morph(Hatch): returns NULL.";
		return;
	}
	waittill = hatch;
}

void ZergStrategieCode::onTick()
{
	if (Broodwar->self()->getRace() != Races::Zerg)
		return;

	if ((waittill == NULL) || (waittill->time == 0)) {
		release(waittill);
		
		for (int k=0; k<10; ++k) {
			waittill = morphUnit(UnitTypes::Zerg_Zergling);
			if (waittill == NULL) {
				LOG << "Error in morph(Zergling): returns NULL.";
				return;
			}
		}
	}
}
