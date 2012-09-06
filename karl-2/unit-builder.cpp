#include "unit-builder.hpp"
#include "building-placer.hpp"
#include "resources.hpp"
#include "mineral-line.hpp"
#include "precondition-helper.hpp"
#include "vector-helper.hpp"
#include "utils/debug.h"
#include <algorithm>
#include <cassert>

using namespace BWAPI;

namespace
{
	struct UnitBuilderPrecondition : public UnitPrecondition
	{
		enum StatusType { pending, commanded, waiting, finished };

		UnitPrecondition*       		baseunit;
		ResourcesPrecondition*  		resources;
		BuildingPositionPrecondition* 	pos;
		StatusType 						status;
		UnitPrecondition*				postworker;

		UnitBuilderPrecondition(UnitPrecondition* u, ResourcesPrecondition* r, BuildingPositionPrecondition* p, const UnitType& ut)
			: UnitPrecondition(ut, u->pos, u->unit), baseunit(u), resources(r), pos(p), status(pending), postworker(NULL)
		{
			updateTime();
			if (ut.getRace() == Races::Terran) {
				postworker = new UnitPrecondition(time, baseunit->ut, Position(pos->pos));
			} else if (ut.getRace() == Races::Protoss) {
				postworker = new UnitPrecondition(time - ut.buildTime(), baseunit->ut, Position(pos->pos));
			} else {
				// Zerg worker is consumed.
			}
		}
		
		~UnitBuilderPrecondition()
		{
			free(baseunit);
			free(resources);
			free(pos);
		}

		bool updateTime()
		{
			switch (status)
			{
				case pending:
					if (updateTimePreconditions(this, ut.buildTime(), baseunit, resources, pos)) {
						start();
						time = Broodwar->getFrameCount() + ut.buildTime();
						status = commanded;
						LOG << "building started.";
					}
					break;

				case commanded:
					time = Broodwar->getFrameCount() + ut.buildTime();
					if (hasStarted()) {
						freeResources();
						status = waiting;
						LOG << "waiting for building to finish.";
					}
					break;

				case waiting:
					if (isFinished()) {
						time   = 0;
						status = finished;
						LOG << "building finished.";
					}
					break;
				
				case finished:
					break;
			}
			return (status == finished);
		}

		void start()
		{
			Unit* worker = NULL;
			if (baseunit != NULL) {
				worker = baseunit->unit;
				if ((ut.getRace() == Races::Terran) || (ut.getRace() == Races::Protoss)) {
					postworker->unit = worker;
				} else if (ut.getRace() == Races::Zerg) {
					unit = worker;
				}
				free(baseunit);
			}
			assert(worker != NULL);
			if (!unit->build(pos->pos, ut))
				LOG << "Error: Unable to morph unit, internal bug!";
		}
		
		bool hasStarted() const
		{
			if (ut.getRace() != Races::Zerg)
				return (unit != NULL); //(postworker->getLastCommand().getType() == BWAPI::UnitCommandTypes::Build);
			else
				return (unit->getType() == ut);
		}
		
		bool isFinished() const
		{
			return !unit->isBeingConstructed();
		}

		void freeResources()
		{
			free(resources);
			free(pos);
		}
		
		bool onAssignUnit(Unit* unit)
		{
			if (status != commanded)
				return false;
			if (unit->getType() != ut)
				return false;
			if (unit->getTilePosition() != pos->pos)
				return false;
			return true;
		}
	};

	std::vector<UnitBuilderPrecondition*> list;
}

std::pair<UnitPrecondition*, UnitPrecondition*> buildUnit(UnitPrecondition* worker, ResourcesPrecondition* res, BuildingPositionPrecondition* pos, const BWAPI::UnitType& ut)
{
	UnitBuilderPrecondition* result = new UnitBuilderPrecondition(worker, res, pos, ut);
    list.push_back(result);
    return std::make_pair(result, result->postworker);
}

std::pair<UnitPrecondition*, UnitPrecondition*> buildUnit(UnitPrecondition* worker, BuildingPositionPrecondition* pos, const BWAPI::UnitType& ut)
{
	ResourcesPrecondition* res = getResources(ut);
	if (res == NULL)
		return std::pair<UnitPrecondition*, UnitPrecondition*>(NULL, NULL);
	return buildUnit(worker, res, pos, ut);
}

std::pair<UnitPrecondition*, UnitPrecondition*> buildUnit(BuildingPositionPrecondition* pos, const BWAPI::UnitType& ut)
{
	UnitPrecondition* worker = getWorker(ut.getRace());
	if (worker == NULL)
		return std::pair<UnitPrecondition*, UnitPrecondition*>(NULL, NULL);
	return buildUnit(worker, pos, ut);
}

std::pair<UnitPrecondition*, UnitPrecondition*> buildUnit(const BWAPI::UnitType& ut)
{
	BuildingPositionPrecondition* pos = getBuildingPosition(ut);
	if (pos == NULL)
		return std::pair<UnitPrecondition*, UnitPrecondition*>(NULL, NULL);
	return buildUnit(pos, ut);
}

void UnitBuilderCode::onMatchEnd()
{
	VectorHelper::clear_and_delete(list);
}

void UnitBuilderCode::onTick()
{
	VectorHelper::remove_if(list, std::mem_fun(&UnitBuilderPrecondition::updateTime));
}

bool UnitBuilderCode::onAssignUnit(BWAPI::Unit* unit)
{
	for (auto it : list)
		if (it->onAssignUnit(unit))
			return true;
	return false;
}
