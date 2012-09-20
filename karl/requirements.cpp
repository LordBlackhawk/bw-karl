// ToDo:
//  *

#include "requirements.hpp"
#include "vector-helper.hpp"
#include "precondition-helper.hpp"
#include "unit-observer.hpp"
#include "object-counter.hpp"
#include "idle-unit-container.hpp"
#include "utils/debug.h"
#include <map>
#include <vector>
#include <algorithm>
#include <functional>

using namespace BWAPI;
using namespace std::placeholders;

namespace
{
	struct RequirementHandler;
	
	struct ReqUnitInternal : public UnitObserver<ReqUnitInternal>, public ObjectCounter<ReqUnitInternal>
	{
		RequirementHandler* handler;
		
		ReqUnitInternal(RequirementHandler* h, UnitPrecondition* p);
		void onRemoveFromList();
		void onFulfilled();
	};
	
	struct RequirementHandler : public ObjectCounter<RequirementHandler>
	{
		int 							time;
		std::set<Unit*>					finished;
		std::vector<ReqUnitInternal*>  	planed;
		
		RequirementHandler()
			: time(1)
		{ }
		
		~RequirementHandler()
		{
			for (auto it : planed)
				it->handler = NULL;
		}
		
		void update(const UnitType& ut);
		
		UnitPrecondition* registerRequirement(UnitPrecondition* unit)
		{
			return new ReqUnitInternal(this, unit);
		}
		
		void onUnitDestroy(Unit* unit)
		{
			finished.erase(unit);
		}
		
		void autoBuild(const UnitType& ut)
		{
			if ((time > Precondition::Max) && planed.empty()) {
				LOG << "Building requirement " << ut.getName() << ".";
				rememberIdle(createUnit(ut));
			}
		}
	};
	
	ReqUnitInternal::ReqUnitInternal(RequirementHandler* h, UnitPrecondition* p)
		: UnitObserver<ReqUnitInternal>(p), handler(h)
	{
		assert(handler != NULL);
		handler->planed.push_back(this);
	}
	
	void ReqUnitInternal::onRemoveFromList()
	{
		if (handler != NULL)
			VectorHelper::remove(handler->planed, this);
	}
	
	void ReqUnitInternal::onFulfilled()
	{
		assert(handler != NULL);
		assert(unit != NULL);
		handler->finished.insert(unit);
	}
	
	void RequirementHandler::update(const UnitType& /*ut*/)
	{
		VectorHelper::remove_if(planed, std::mem_fun(&ReqUnitInternal::update));
		
		if (!finished.empty()) {
			time = 0;
		} else if (planed.empty()) {
			time = Precondition::Impossible;
		} else {
			time = (*planed.begin())->time;
			for (auto it : planed)
				time = std::min(time, it->time);
		}
	}
	
	std::map<UnitType, RequirementHandler*>	requirements;
	
	RequirementHandler* getOrCreateHandler(const UnitType& ut)
	{
		auto it = requirements.find(ut);
		if (it == requirements.end()) {
			RequirementHandler* handler = new RequirementHandler();
			requirements[ut] = handler;
			return handler;
		} else {
			return it->second;
		}
	}
	
	int lookup(const UnitType& tobuild, const UnitType& ut)
	{
		if (ut == UnitTypes::Zerg_Larva)
			return 0;
		if (tobuild.whatBuilds().first == ut)
			return 0;
		auto it = requirements.find(ut);
		if (it == requirements.end()) {
			RequirementHandler* handler = new RequirementHandler();
			requirements[ut] = handler;
			return Precondition::Impossible;
		}
		return it->second->time;
	}
	
	struct RequirementsInternal;
	
	std::set<RequirementsInternal*> reqlist;
	
	struct RequirementsInternal : public RequirementsPrecondition, public ObjectCounter<RequirementsInternal>
	{
		RequirementsInternal(const UnitType& t)
			: RequirementsPrecondition(t)
		{
			reqlist.insert(this);
		}
		
		~RequirementsInternal()
		{
			reqlist.erase(this);
		}
		
		void update()
		{
			time = 0;
			for (auto it : ut.requiredUnits()) {
				int newtime = lookup(ut, it.first);
				time = std::max(time, newtime);
			}
		}
	};
	
	int mode = RequirementsMode::None;
}

RequirementsPrecondition* getRequirements(const BWAPI::UnitType& t)
{
	return new RequirementsInternal(t);
}

bool isRequirement(const BWAPI::UnitType& t)
{
	if (!t.isBuilding())
		return false;
	if (   (t == UnitTypes::Terran_Supply_Depot)
	    || (t == UnitTypes::Terran_Refinery)
		|| (t == UnitTypes::Terran_Bunker)
		|| (t == UnitTypes::Terran_Missile_Turret)
		
		|| (t == UnitTypes::Zerg_Creep_Colony)
		|| (t == UnitTypes::Zerg_Spore_Colony)
		|| (t == UnitTypes::Zerg_Sunken_Colony)
		|| (t == UnitTypes::Zerg_Extractor)
		
		|| (t == UnitTypes::Protoss_Pylon)
		|| (t == UnitTypes::Protoss_Assimilator)
		|| (t == UnitTypes::Protoss_Photon_Cannon))
	{
		return false;
	}
	return true;
}

UnitPrecondition* registerRequirement(UnitPrecondition* unit)
{
	return getOrCreateHandler(unit->ut)->registerRequirement(unit);
}

void setRequirementsMode(int m)
{
	mode = m;
}

void RequirementsCode::onMatchBegin()
{
	for (auto it : Broodwar->self()->getUnits()) {
		UnitType ut = it->getType();
		if (ut == UnitTypes::Zerg_Hatchery)
			getOrCreateHandler(ut)->finished.insert(it);
	}
}

void RequirementsCode::onMatchEnd()
{
	VectorHelper::clear_and_delete_second(requirements);
	reqlist.clear();
}

void RequirementsCode::onTick()
{
	for (auto it : requirements)
		it.second->update(it.first);
	for (auto it : reqlist)
		it->update();
	
	switch (mode)
	{
		case RequirementsMode::Auto:
			for (auto it : requirements)
				it.second->autoBuild(it.first);
			break;
		default:
			break;
	}
}

void RequirementsCode::onUnitDestroy(BWAPI::Unit* unit)
{
	for (auto it : requirements)
		it.second->onUnitDestroy(unit);
}

void RequirementsCode::onCheckMemoryLeaks()
{
	ReqUnitInternal::checkObjectsAlive();
	RequirementHandler::checkObjectsAlive();
	RequirementsInternal::checkObjectsAlive();
}
