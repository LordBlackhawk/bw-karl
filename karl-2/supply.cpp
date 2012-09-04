// ToDo:
//  * Terran/Protoss build supply not jet implemented.

#include "supply.hpp"
#include "vector-helper.hpp"
#include "unit-morpher.hpp"
#include "precondition-helper.hpp"
#include "utils/debug.h"
#include <BWAPI.h>

using namespace BWAPI;

namespace
{
	struct SupplyUnitPrecondition;
	
	struct RaceSupply
	{
		BWAPI::Race								race;
		std::vector<SupplyPrecondition*>		supply;
		std::vector<SupplyUnitPrecondition*>	supplyunits;
		int										remaining;
		int										remaining_time;
		
		void init(const BWAPI::Race& r)
		{
			race           = r;
			Player* self   = Broodwar->self();
			remaining      = self->supplyTotal(race) - self->supplyUsed(race);
			remaining_time = 0;
		}
		
		void reset()
		{
			supply.clear();
			supplyunits.clear();
		}
		
		void calcSupplyUnit(SupplyUnitPrecondition* unit);
		void addSupplyUnit(SupplyUnitPrecondition* unit);
		
		void fillSupply()
		{
			while (remaining < 0) {
				int oldremaining = remaining;
				if (buildSupply() == NULL)
					LOG << "build Supply returns NULL";
				if (remaining <= oldremaining) {
					LOG << "remaining does not increase after buildSupply()";
					return;
				}
			}
		}
		
		void addSupply(SupplyPrecondition* s)
		{
			supply.push_back(s);
			remaining -= s->supply;
			fillSupply();
			s->time = remaining_time;
		}
		
		void removeSupply(SupplyPrecondition* s)
		{
			VectorHelper::remove(supply, s);
		}
		
		UnitPrecondition* buildSupply();
		void onTick();
	};
	
	RaceSupply terran_supply;
	RaceSupply protoss_supply;
	RaceSupply zerg_supply;

	struct SupplyUnitPrecondition : public UnitPrecondition
	{
		UnitPrecondition* pre;
		
		SupplyUnitPrecondition(UnitPrecondition* p)
			: UnitPrecondition(p->ut, p->pos, p->unit), pre(p)
		{
			update();
			Race r = ut.getRace();
			if (r == Races::Terran) {
				terran_supply.addSupplyUnit(this);
			} else if (r == Races::Protoss) {
				protoss_supply.addSupplyUnit(this);
			} else if (r == Races::Zerg) {
				zerg_supply.addSupplyUnit(this);
			}
		}
		
		bool update()
		{
			pre->wishtime = wishtime;
			time          = pre->time;
			if (pre->time == 0)
				return fire();
			return false;
		}
		
		bool fire()
		{
			unit = pre->unit;
			free(pre);
			return true;
		}
	};
	
	struct SupplyPreconditionInternal : public SupplyPrecondition
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
	
	void RaceSupply::calcSupplyUnit(SupplyUnitPrecondition* unit)
	{
		remaining     += unit->ut.supplyProvided();
		remaining_time = unit->time;
	}
	
	void RaceSupply::addSupplyUnit(SupplyUnitPrecondition* unit)
	{
		supplyunits.push_back(unit);
		calcSupplyUnit(unit);
	}
	
	void RaceSupply::onTick()
	{
		VectorHelper::remove_if(supplyunits, std::mem_fun(&SupplyUnitPrecondition::update));
		std::sort(supplyunits.begin(), supplyunits.end(), PreconditionSorter());
		
		Player* self   = Broodwar->self();
		remaining      = self->supplyTotal(race) - self->supplyUsed(race);
		remaining_time = 0;
		auto uit       = supplyunits.begin();
		auto uitend    = supplyunits.end();
		
		for (auto it : supply) {
			remaining -= it->supply;
			while ((remaining < 0) && (uit != uitend)) {
				calcSupplyUnit(*uit);
				++uit;
			}
			if (remaining < 0) {
				fillSupply();
				uit = uitend = supplyunits.end();
			}
			it->time = remaining_time;
		}
		
		for (; uit!=uitend; ++uit)
			calcSupplyUnit(*uit);
	}
	
	UnitPrecondition* RaceSupply::buildSupply()
	{
		LOG << "buildSupply called.";

		UnitPrecondition* result = NULL;
		if (race == Races::Terran) {
			LOG << "Not implemented!!!";
			exit(1);
		} else if (race == Races::Protoss) {
			LOG << "Not implemented!!!";
			exit(1);
		} else if (race == Races::Zerg) {
			result = morphUnit(UnitTypes::Zerg_Overlord);
			if (result == NULL)
				LOG << "Not able to build overlord!";
		}
		return result;
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
	return new SupplyUnitPrecondition(unit);
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
