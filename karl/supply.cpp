// ToDo:
//  * Bring SupplyUnits with wishtime at end of list.
//  * Bug: Building SupplyUnit at "just in supply". Solution: Add supply units only after n ticks.
//  * Supply blocked units are always behind buildings. Problem?

#include "supply.hpp"
#include "container-helper.hpp"
#include "unit-morpher.hpp"
#include "unit-builder.hpp"
#include "mineral-line.hpp"
#include "idle-unit-container.hpp"
#include "precondition-helper.hpp"
#include "object-counter.hpp"
#include "unit-observer.hpp"
#include "log.hpp"
#include <BWAPI.h>
#include <cassert>

using namespace BWAPI;

#define THIS_DEBUG DEBUG

namespace
{
	struct SupplyUnitObserver;
	
	struct RaceSupply
	{
		BWAPI::Race								race;
		std::vector<SupplyPrecondition*>		supply;
		std::vector<SupplyUnitObserver*>		supplyunits;
		int										remaining;
		int										remaining_time;
		int										mode;
		
		void init(const BWAPI::Race& r)
		{
			race           = r;
			Player* self   = Broodwar->self();
			remaining      = self->supplyTotal(race) - self->supplyUsed(race);
			remaining_time = 0;
			mode           = SupplyMode::None;
		}
		
		void reset()
		{
			supply.clear();
			supplyunits.clear();
		}
		
		void calcSupplyUnit(SupplyUnitObserver* unit);
		void addSupplyUnit(SupplyUnitObserver* unit);
		void removeSupplyUnit(SupplyUnitObserver* unit);
		
		void addSupply(SupplyPrecondition* s)
		{
			supply.push_back(s);
			remaining -= s->supply;
			//fillSupply();
			if (remaining < 0)
				remaining_time = Precondition::Impossible;
			s->time = remaining_time;
		}
		
		void removeSupply(SupplyPrecondition* s)
		{
			Containers::remove(supply, s);
		}
		
		void buildSupply()
		{
			THIS_DEBUG << "buildSupply called.";
			if (race == Races::Terran) {
				auto result = buildUnit(UnitTypes::Terran_Supply_Depot);
				rememberIdle(result.first);
				useWorker(result.second);
			} else if (race == Races::Protoss) {
				auto result = buildUnit(UnitTypes::Protoss_Pylon);
				rememberIdle(result.first);
				useWorker(result.second);
			} else if (race == Races::Zerg) {
				rememberIdle(morphUnit(UnitTypes::Zerg_Overlord));
			}
		}
		
		void onTick();
	};
	
	RaceSupply terran_supply;
	RaceSupply protoss_supply;
	RaceSupply zerg_supply;
	
	struct SupplyUnitObserver : public UnitObserver<SupplyUnitObserver>, public ObjectCounter<SupplyUnitObserver>
	{
		SupplyUnitObserver(UnitPrecondition* p)
			: UnitObserver<SupplyUnitObserver>(p)
		{
			Race r = ut.getRace();
			if (r == Races::Terran) {
				terran_supply.addSupplyUnit(this);
			} else if (r == Races::Protoss) {
				protoss_supply.addSupplyUnit(this);
			} else if (r == Races::Zerg) {
				zerg_supply.addSupplyUnit(this);
			}
		}
	
		void onRemoveFromList()
		{
			Race r = ut.getRace();
			if (r == Races::Terran) {
				terran_supply.removeSupplyUnit(this);
			} else if (r == Races::Protoss) {
				protoss_supply.removeSupplyUnit(this);
			} else if (r == Races::Zerg) {
				zerg_supply.removeSupplyUnit(this);
			}
		}
	};
	
	struct SupplyPreconditionInternal : public SupplyPrecondition, public ObjectCounter<SupplyPreconditionInternal>
	{
		SupplyPreconditionInternal(const BWAPI::Race& r, int s)
			: SupplyPrecondition(r, s)
		{
			if (race == Races::Terran) {
				terran_supply.addSupply(this);
			} else if (race == Races::Protoss) {
				protoss_supply.addSupply(this);
			} else if (race == Races::Zerg) {
				zerg_supply.addSupply(this);
			}
		}
		
		~SupplyPreconditionInternal()
		{
			if (race == Races::Terran) {
				terran_supply.removeSupply(this);
			} else if (race == Races::Protoss) {
				protoss_supply.removeSupply(this);
			} else if (race == Races::Zerg) {
				zerg_supply.removeSupply(this);
			}
		}
	};
	
	struct SupplySorter
	{
		bool operator () (SupplyPrecondition* lhs, SupplyPrecondition* rhs)
		{
			return std::max(lhs->time, lhs->wishtime) < std::max(rhs->time, rhs->wishtime);
		}
	};
	
	void RaceSupply::calcSupplyUnit(SupplyUnitObserver* unit)
	{
		remaining     += unit->ut.supplyProvided();
		remaining_time = unit->time;
	}
	
	void RaceSupply::addSupplyUnit(SupplyUnitObserver* unit)
	{
		supplyunits.push_back(unit);
		calcSupplyUnit(unit);
	}
	
	void RaceSupply::removeSupplyUnit(SupplyUnitObserver* unit)
	{
		Containers::remove(supplyunits, unit);
	}
	
	void RaceSupply::onTick()
	{
		Containers::remove_if(supplyunits, std::mem_fun(&SupplyUnitObserver::update));
		std::stable_sort(supplyunits.begin(), supplyunits.end(), PreconditionSorter());
		
		Player* self   = Broodwar->self();
		remaining      = self->supplyTotal(race) - self->supplyUsed(race);
		remaining_time = 0;
		auto uit       = supplyunits.begin();
		auto uitend    = supplyunits.end();
		
		std::stable_sort(supply.begin(), supply.end(), SupplySorter());
		for (auto it : supply) {
			remaining -= it->supply;
			while ((remaining < 0) && (uit != uitend)) {
				calcSupplyUnit(*uit);
				++uit;
			}
			if (remaining < 0) {
				remaining_time = Precondition::Impossible;
				//fillSupply();
				//uit = uitend = supplyunits.end();
			}
			it->time = remaining_time;
		}
		
		for (; uit!=uitend; ++uit)
			calcSupplyUnit(*uit);
		
		switch (mode)
		{
			case SupplyMode::Auto:
				if (remaining < 0)
					buildSupply();
				break;
			
			default:
				break;
		}
	}
}

SupplyPrecondition* getSupply(const BWAPI::Race& r, int s)
{
	if (s <= 0)
		return NULL;
	return new SupplyPreconditionInternal(r, s);
}

SupplyPrecondition* getSupply(const BWAPI::UnitType& ut)
{
	return getSupply(ut.getRace(), ut.supplyRequired());
}

UnitPrecondition* registerSupplyUnit(UnitPrecondition* unit)
{
	return new SupplyUnitObserver(unit);
}

void setSupplyMode(const BWAPI::Race& r, int mode)
{
	if (r == Races::Terran) {
		terran_supply.mode = mode;
	} else if (r == Races::Protoss) {
		protoss_supply.mode = mode;
	} else if (r == Races::Zerg) {
		zerg_supply.mode = mode;
	}
}

void SupplyCode::onMatchBegin()
{
	terran_supply.init(Races::Terran);
	protoss_supply.init(Races::Protoss);
	zerg_supply.init(Races::Zerg);
}

void SupplyCode::onMatchEnd()
{
	terran_supply.reset();
	protoss_supply.reset();
	zerg_supply.reset();
}

void SupplyCode::onTick()
{
	terran_supply.onTick();
	protoss_supply.onTick();
	zerg_supply.onTick();
}

void SupplyCode::onCheckMemoryLeaks()
{
	SupplyUnitObserver::checkObjectsAlive();
	SupplyPreconditionInternal::checkObjectsAlive();
}
