// ToDo:
//  * 

#include "idle-unit-container.hpp"
#include "code-list.hpp"
#include "mineral-line.hpp"
#include "larvas.hpp"
#include "unit-morpher.hpp"
#include "unit-builder.hpp"
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
	
	std::vector<UnitPrecondition*>	waitingfor;
	
	bool updateWaiting(UnitPrecondition* pre)
	{
		if (pre->isFulfilled()) {
			idleunits.insert(pre->unit);
			delete pre;
			return true;
		}
		return false;
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

void rememberIdle(UnitPrecondition* unit)
{
	if (unit == NULL)
		return;

	waitingfor.push_back(unit);
}

UnitPrecondition* createUnit(const BWAPI::UnitType& ut)
{
	if (ut.whatBuilds().first.isWorker()) {
		auto result = buildUnit(ut);
		useWorker(result.second);
		return result.first;
	} else if (ut.getRace() == Races::Zerg) {
		return morphUnit(ut);
	} else {
		return NULL;
	}
}

void IdleUnitContainerCode::onMatchEnd()
{
	idleunits.clear();
	VectorHelper::clear_and_delete(waitingfor);
}

void IdleUnitContainerCode::onTick()
{
	VectorHelper::remove_if(waitingfor, updateWaiting);
}

void IdleUnitContainerCode::onUnitCreate(BWAPI::Unit* unit)
{
	if (unit->getPlayer() != Broodwar->self())
		return;
		
	if (CodeList::onAssignUnit(unit))
		return;
	
	if (unit->getType().isWorker() && (Broodwar->getFrameCount() == 0))
		return;
	
	idleunits.insert(unit);
}

void IdleUnitContainerCode::onUnitDestroy(BWAPI::Unit* unit)
{
	Player* self = Broodwar->self();
	if (unit->getPlayer() != self)
		return;
	
	idleunits.erase(unit);
}