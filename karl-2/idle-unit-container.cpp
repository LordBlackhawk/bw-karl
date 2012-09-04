// ToDo:
//  * Improving assignment of larvas.

#include "idle-unit-container.hpp"
#include "utils/debug.h"
#include <algorithm>
#include <functional> 
#include <set>
#include <vector>

using namespace BWAPI;
using namespace std::placeholders;

namespace
{
	std::set<Unit*> 				idleunits;
	std::vector<UnitPrecondition*>	reservedunits;

	UnitPrecondition* createAndErase(Unit* unit)
	{
		idleunits.erase(unit);
		return new UnitPrecondition(unit->getType(), unit->getPosition(), unit);
	}

	UnitPrecondition* createAndReserve(const UnitType& ut)
	{
		UnitPrecondition* result = new UnitPrecondition(ut, Position(1, 1), NULL);
		reservedunits.push_back(result);
		result->time = 300;
		return result;
	}
	
	bool isOfType(UnitPrecondition* unit, UnitType ut)
	{
		return (unit->ut == ut);
	}
}

UnitPrecondition* getIdleUnit(const BWAPI::UnitType& ut)
{
	for (auto it : idleunits)
		if (it->getType() == ut)
			return createAndErase(it);
	if (ut == UnitTypes::Zerg_Larva)
		return createAndReserve(ut);
	return NULL;
}

void IdleUnitContainerCode::onMatchBegin()
{
	/*
	Player* self = Broodwar->self();
	for (auto it : self->getUnits())
		if (!it->getType().isWorker())
			idleunits.insert(it);
	*/
}

void IdleUnitContainerCode::onMatchEnd()
{
	idleunits.clear();
	reservedunits.clear();
}

void IdleUnitContainerCode::onUnitCreate(BWAPI::Unit* unit)
{
	if (unit->getPlayer() != Broodwar->self())
		return;
	if (unit->getType() == UnitTypes::Zerg_Drone)
		return;

	auto it = std::find_if(reservedunits.begin(), reservedunits.end(), std::bind(isOfType, _1, unit->getType()));
	if (it == reservedunits.end()) {
		LOG << "Unit " << unit->getType().getName() << " added to idle units.";
		idleunits.insert(unit);
	} else {
		LOG << "Unit " << unit->getType().getName() << " was reserved.";
		(*it)->unit = unit;
		(*it)->time = 0;
		reservedunits.erase(it);
	}
}

void IdleUnitContainerCode::onUnitDestroy(BWAPI::Unit* unit)
{
	idleunits.erase(unit);
}
