// ToDo:
//  * 

#include "unit-morpher.hpp"
#include "precondition-helper.hpp"
#include "resources.hpp"
#include "supply.hpp"
#include "idle-unit-container.hpp"
#include "vector-helper.hpp"
#include "debugger.hpp"
#include "utils/debug.h"
#include <BWAPI.h>
#include <vector>
#include <cassert>
#include <algorithm>

using namespace BWAPI;

namespace
{
	struct UnitMorphPrecondition : public UnitPrecondition
	{
		enum StatusType { pending, commanded, waiting, finished };

		UnitPrecondition*       baseunit;
		ResourcesPrecondition*  resources;
		SupplyPrecondition*		supply;
		StatusType 				status;

		UnitMorphPrecondition(UnitPrecondition* u, ResourcesPrecondition* r, SupplyPrecondition* s, UnitType ut)
			: UnitPrecondition(ut, u->pos, u->unit), baseunit(u), resources(r), supply(s), status(pending)
		{
			updateTime();
		}
		
		~UnitMorphPrecondition()
		{
			free(baseunit);
			free(resources);
			free(supply);
		}

		bool updateTime()
		{	
			switch (status)
			{
				case pending:
					if (updateTimePreconditions(this, ut.buildTime(), baseunit, resources, supply)) {
						start();
						time = Broodwar->getFrameCount() + ut.buildTime();
						status = commanded;
					}
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
				free(baseunit);
			}
			assert(unit != NULL);
			assert(unit->getType() == UnitTypes::Zerg_Larva);
			if (!unit->morph(ut))
				LOG << "Error: Unable to morph unit, internal bug!";
		}

		void freeResources()
		{
			free(resources);
			free(supply);
		}
	};

	std::vector<UnitMorphPrecondition*> list;
}

void UnitMorpherCode::onMatchEnd()
{
	VectorHelper::clear_and_delete(list);
}

void UnitMorpherCode::onTick()
{
	VectorHelper::remove_if(list, std::mem_fun(&UnitMorphPrecondition::updateTime));
}

UnitPrecondition* morphUnit(UnitPrecondition* unit, ResourcesPrecondition* res, SupplyPrecondition* supply, const BWAPI::UnitType& ut, const char* debugname)
{
	UnitMorphPrecondition* result = new UnitMorphPrecondition(unit, res, supply, ut);
    list.push_back(result);
	if (ut != UnitTypes::Zerg_Overlord)
		return result;
	if (debugname != NULL)
		setDebugName(result, debugname);
    return registerSupplyUnit(result);
}

UnitPrecondition* morphUnit(UnitPrecondition* unit, ResourcesPrecondition* res, const BWAPI::UnitType& ut, const char* debugname)
{
    SupplyPrecondition* supply = getSupply(ut);
	// supply maybe NULL.
	if (debugname != NULL)
		setDebugName(supply, debugname);
	return morphUnit(unit, res, supply, ut, debugname);
}

UnitPrecondition* morphUnit(UnitPrecondition* unit, const BWAPI::UnitType& ut, const char* debugname)
{
    ResourcesPrecondition* res = getResources(ut);
	if (res == NULL)
		return NULL;
	if (debugname != NULL)
		setDebugName(res, debugname);
    return morphUnit(unit, res, ut, debugname);
}

UnitPrecondition* morphUnit(const BWAPI::UnitType& ut, const char* debugname)
{
    UnitPrecondition* unit = getIdleUnit(ut.whatBuilds().first);
	if (unit == NULL)
		return NULL;
	if (debugname != NULL)
		setDebugName(unit, std::string(debugname) + " larva");
    return morphUnit(unit, ut, debugname);
}
