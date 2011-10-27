#pragma once

#include "bwplan.h"
#include <BWAPI.h>

template <class RT, class RTGas>
void addworker(BWResources& res, int time, const BWAPI::Unit* u)
{
	res.inc<RT>(time);
	if (!u->isGatheringMinerals())
	{
		res.incLocked<RT>(time);
		if (u->isGatheringGas()) {
			res.inc<RTGas>(time);
			res.incLocked<RGeyserWorkPlace>(time);
		}
	}
}

BWResources extractResources()
{
	BWResources res;
	int time = Broodwar->getFrameCount();
	res.setTime(time);

	BWAPI::Player* self = Broodwar->self();

	res.set<RMinerals>(self->minerals());
	res.set<RGas>(self->gas());

	res.set<RTerranSupply>(self->supplyTotal(BWAPI::Races::Terran));
	res.setLocked<RTerranSupply>(self->supplyUsed(BWAPI::Races::Terran));

	res.set<RProtossSupply>(self->supplyTotal(BWAPI::Races::Protoss));
	res.setLocked<RProtossSupply>(self->supplyUsed(BWAPI::Races::Protoss));

	res.set<RZergSupply>(self->supplyTotal(BWAPI::Races::Zerg));
	res.setLocked<RZergSupply>(self->supplyUsed(BWAPI::Races::Zerg));));

	for (auto it : self->getUnits())
	{
		BWAPI::UnitType& ut = it->getType();

		// Worker:
		if (ut.isWorker()) {
			if (ut.getRace() == BWAPI::Races::Terran) {
				addworker< RTerranWorker, RTerranGasWorker >(res, time, it);
			} else if (ut.getRace() == BWAPI::Races::Protoss) {
				addworker< RProtossWorker, RProtossGasWorker >(res, time, it);
			} else if (ut.getRace() == BWAPI::Races::Zerg) {
				addworker< RZergWorker, RZergGasWorker >(res, time, it);
			}
			continue;
		}

		// Larva:
		if (ut == BWAPI::UnitTypes::Zerg_Larva) {
			res.inc<RLarva>(time);
			continue;
		}

		// Refinery:
		if (ut.isRefinery()) {
			res.inc<RGeyserWorkPlace>(time, 3);
			continue;
		}

		// Other Buildings:
		if (!ut.isBuilding())
			continue;

		if (it->isBeingConstructed())
			continue;

		BWResourceIndex ri = BWResourceIndex::byAssociation<BWAPI::UnitType>(ut);
		res.inc(ri, time);
		if (!it->isIdle())
			res.incLocked(ri, time);
	}

	return res;
}
