// ToDo:
//  * 

#include "idle-unit-container.hpp"
#include "code-list.hpp"
#include "mineral-line.hpp"
#include "larvas.hpp"
#include "vector-helper.hpp"
#include "utils/debug.h"
#include <algorithm>
#include <vector>

using namespace BWAPI;

namespace
{
	std::set<Unit*> 				idleunits;

	UnitPrecondition* createAndErase(Unit* unit)
	{
		idleunits.erase(unit);
		return new UnitPrecondition(unit);
	}
}

UnitPrecondition* getIdleUnit(const BWAPI::UnitType& ut)
{
	if (ut == UnitTypes::Zerg_Larva)
		return getLarva();
	for (auto it : idleunits)
		if (it->getType() == ut)
			return createAndErase(it);
	return NULL;
}

void IdleUnitContainerCode::onMatchEnd()
{
	idleunits.clear();
}

void IdleUnitContainerCode::onTick()
{ }

void IdleUnitContainerCode::onUnitCreate(BWAPI::Unit* unit)
{
	if (unit->getPlayer() != Broodwar->self())
		return;
		
	if (CodeList::onAssignUnit(unit))
		return;
	
	if (unit->getType().isWorker()) {
		useWorker(new UnitPrecondition(unit));
		return;
	}
	
	idleunits.insert(unit);
}

void IdleUnitContainerCode::onUnitDestroy(BWAPI::Unit* unit)
{
	Player* self = Broodwar->self();
	if (unit->getPlayer() != self)
		return;
	
	idleunits.erase(unit);
}
