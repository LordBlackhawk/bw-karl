#pragma once

#include "bwplan.h"
#include <BWAPI.h>

void addworker(BWResources& res, int time, const BWAPI::Unit* u, const ResourceIndex& riWorker, const ResourceIndex& riGasWorker)
{
	res.inc(riWorker, time, 1);
	if (!u->isGatheringMinerals())
	{
		res.incLocked(riWorker, time, 1);
		if (u->isGatheringGas()) {
			res.inc(riGasWorker, time, 1);
			res.incLocked(BWResourceIndex::GasWorkingPlaces, time, 1);
		}
	}
}

BWResources extractResources()
{
	BWResources res;
	int time = BWAPI::Broodwar->getFrameCount();
	res.setTime(time);

	BWAPI::Player* self = BWAPI::Broodwar->self();

	res.set(BWResourceIndex::Minerals, self->minerals());
	res.set(BWResourceIndex::Gas, self->gas());

	res.set(BWResourceIndex::TerranSupply, self->supplyTotal(BWAPI::Races::Terran) - self->supplyUsed(BWAPI::Races::Terran));
	res.setLocked(BWResourceIndex::TerranSupply, self->supplyUsed(BWAPI::Races::Terran));

	res.set(BWResourceIndex::ProtossSupply, self->supplyTotal(BWAPI::Races::Protoss) - self->supplyUsed(BWAPI::Races::Protoss));
	res.setLocked(BWResourceIndex::ProtossSupply, self->supplyUsed(BWAPI::Races::Protoss));

	res.set(BWResourceIndex::ZergSupply, self->supplyTotal(BWAPI::Races::Zerg) - self->supplyUsed(BWAPI::Races::Zerg));
	res.setLocked(BWResourceIndex::ZergSupply, self->supplyUsed(BWAPI::Races::Zerg));

	for (auto it : self->getUnits())
	{
		BWAPI::UnitType ut = it->getType();

		// Worker:
		if (ut.isWorker()) {
			if (ut.getRace() == BWAPI::Races::Terran) {
				addworker(res, time, it, BWResourceIndex::TerranWorker, BWResourceIndex::TerranGasWorker);
			} else if (ut.getRace() == BWAPI::Races::Protoss) {
				addworker(res, time, it, BWResourceIndex::ProtossWorker, BWResourceIndex::ProtossGasWorker);
			} else if (ut.getRace() == BWAPI::Races::Zerg) {
				addworker(res, time, it, BWResourceIndex::ZergWorker, BWResourceIndex::ZergGasWorker);
			}
			continue;
		}

		// Larva:
		if (ut == BWAPI::UnitTypes::Zerg_Larva) {
			res.inc(BWResourceIndex::Larva, time, 1);
			continue;
		}

		// Other Buildings:
		if (!ut.isBuilding())
			continue;
			
		// Ein Hive ist auch ein Lair, damit das mit dem Forschen klappt.
		if (ut == BWAPI::UnitTypes::Zerg_Hive) {
			res.inc(BWResourceIndex::ZergLair, time, 1);
			if (it->isBeingConstructed() || it->isUpgrading() || it->isResearching())
				res.inc(BWResourceIndex::ZergLair, time, 1);
		}

		if (it->isBeingConstructed())
			continue;
		
		// Refinery:
		if (ut.isRefinery()) {
			res.inc(BWResourceIndex::GasWorkingPlaces, time, 3);
			continue;
		}

		BWResourceIndex ri = BWResourceIndex::byUnitType(ut);
		res.inc(ri, time, 1);
		if (it->isUpgrading() || it->isResearching() || it->isTraining())
			res.incLocked(ri, time, 1);
	}
	
	for (auto ri : TechResourceIndices())
		if (self->hasResearched(ri.associatedTechType()))
			res.inc(ri, time, 1);
	
	for (auto ri : UpgradeResourceIndices())
		res.set(ri, self->getUpgradeLevel(ri.associatedUpgradeType()));

	return res;
}
