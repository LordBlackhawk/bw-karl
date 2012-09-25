// ToDo:
//  *

#include "unit-morpher.hpp"
#include "precondition-helper.hpp"
#include "resources.hpp"
#include "supply.hpp"
#include "idle-unit-container.hpp"
#include "requirements.hpp"
#include "vector-helper.hpp"
#include "object-counter.hpp"
#include "utils/debug.h"
#include <BWAPI.h>
#include <vector>
#include <cassert>
#include <algorithm>

using namespace BWAPI;

namespace
{
	struct UnitMorphPrecondition;
	std::vector<UnitMorphPrecondition*> list;
	
	struct UnitMorphPrecondition : public UnitPrecondition, public ObjectCounter<UnitMorphPrecondition>
	{
		enum StatusType { pending, tryagain, commanded, waiting, finished };

		UnitPrecondition*       	baseunit;
		ResourcesPrecondition*  	resources;
		SupplyPrecondition*			supply;
		RequirementsPrecondition* 	requirements;
		Precondition*				extra;
		StatusType 					status;

		UnitMorphPrecondition(UnitPrecondition* u, ResourcesPrecondition* r, SupplyPrecondition* s, RequirementsPrecondition* v, 
							  const UnitType& ut, Precondition* e)
			: UnitPrecondition(ut, u->pos, u->unit), baseunit(u), resources(r), supply(s), requirements(v), extra(e), status(pending)
		{
			updateTime();
		}
		
		~UnitMorphPrecondition()
		{
			VectorHelper::remove(list, this);

			release(baseunit);
			release(resources);
			release(supply);
			release(requirements);
			release(extra);
		}

		bool updateTime()
		{
			/*if ((Broodwar->getFrameCount() % 100 == 0) && (status == pending)) {
				std::string reason;
				if ((baseunit != NULL) && (baseunit->time != 0))
					reason += " baseunit";
				if ((resources != NULL) && (resources->time != 0))
					reason += " resources";
				if ((supply != NULL) && (supply->time != 0))
					reason += " supply";
				if ((extra != NULL) && (extra->time != 0))
					reason += " extra";
				LOG << "Morphing " << ut.getName() << " waiting for" << reason << ".";
			}*/
			switch (status)
			{
				case pending:
					if (updateTimePreconditions(this, ut.buildTime(), baseunit, resources, supply, requirements, extra)) {
						start();
						time = Broodwar->getFrameCount() + ut.buildTime();
					}
					break;
				
				case tryagain:
					time = Broodwar->getFrameCount() + ut.buildTime();
					start();
					break;

				case commanded:
					time = Broodwar->getFrameCount() + ut.buildTime();
					if (unit->isMorphing()) {
						freeResources();
						status = waiting;
					}
					break;

				case waiting:
					if (!unit->isMorphing()) {
						time   = 0;
						status = finished;
					}
					break;
				
				case finished:
					break;
			}
			return (status == finished);
		}

		void start()
		{
			if (baseunit != NULL) {
				unit = baseunit->unit;
				release(baseunit);
			}
			assert(unit != NULL);
			if (!unit->morph(ut)) {
				Error err = Broodwar->getLastError();
				LOG << "Error: Unable to morph unit " << ut.getName() << ": " << err.toString();
				if (   (err == Errors::Insufficient_Supply)
					|| (err == Errors::Insufficient_Minerals)
					|| (err == Errors::Insufficient_Gas))
				{
					status = pending;
					return;
				}
				
				status = tryagain;
				return;
			}
			
			status = commanded;
		}

		void freeResources()
		{
			release(resources);
			release(supply);
			release(requirements);
			release(extra);
		}
		
		void onDrawPlan()
		{
			if (status == pending)
				return;
			
			assert(unit != NULL);
			Position pos = unit->getPosition();
			int x = pos.x(), y = pos.y();
			Broodwar->drawCircleMap(x, y, 20, Colors::Green, false);
		}
	};
}

UnitPrecondition* morphUnit(UnitPrecondition* unit, ResourcesPrecondition* res, SupplyPrecondition* supply, const BWAPI::UnitType& ut, Precondition* extra)
{
	RequirementsPrecondition* req = getRequirements(ut);
	// req maybe NULL.
	
	UnitMorphPrecondition* result = new UnitMorphPrecondition(unit, res, supply, req, ut, extra);
    list.push_back(result);

	UnitPrecondition* first = result;
	if (ut == UnitTypes::Zerg_Overlord)
		first = registerSupplyUnit(first);
	if (isRequirement(ut))
		first = registerRequirement(first);
    return first;
}

UnitPrecondition* morphUnit(UnitPrecondition* unit, ResourcesPrecondition* res, const BWAPI::UnitType& ut, Precondition* extra)
{
    SupplyPrecondition* supply = getSupply(ut);
	// supply maybe NULL.
	return morphUnit(unit, res, supply, ut, extra);
}

UnitPrecondition* morphUnit(UnitPrecondition* unit, const BWAPI::UnitType& ut, Precondition* extra)
{
    ResourcesPrecondition* res = getResources(ut);
	if (res == NULL)
		return NULL;
    return morphUnit(unit, res, ut, extra);
}

UnitPrecondition* morphUnit(const BWAPI::UnitType& ut, Precondition* extra)
{
    UnitPrecondition* unit = getIdleUnit(ut.whatBuilds().first);
	if (unit == NULL)
		return NULL;
    return morphUnit(unit, ut, extra);
}

void UnitMorpherCode::onMatchEnd()
{
	list.clear();
}

void UnitMorpherCode::onTick()
{
	VectorHelper::remove_if(list, std::mem_fun(&UnitMorphPrecondition::updateTime));
}

void UnitMorpherCode::onDrawPlan()
{
	for (auto it : list)
		it->onDrawPlan();
}

void UnitMorpherCode::onCheckMemoryLeaks()
{
	UnitMorphPrecondition::checkObjectsAlive();
}