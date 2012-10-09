// ToDo:
// *

#include "unit-trainer.hpp"
#include "resources.hpp"
#include "idle-unit-container.hpp"
#include "supply.hpp"
#include "precondition-helper.hpp"
#include "container-helper.hpp"
#include "larvas.hpp"
#include "mineral-line.hpp"
#include "requirements.hpp"
#include "object-counter.hpp"
#include "utils/debug.h"
#include <algorithm>
#include <cassert>

using namespace BWAPI;

namespace
{
	const int savetime = 27;
	
	struct UnitTrainerPrecondition;
	std::vector<UnitTrainerPrecondition*> list;

	struct UnitTrainerPrecondition : public UnitPrecondition, public ObjectCounter<UnitTrainerPrecondition>
	{
		enum StatusType { pending, tryagain, commanded, waiting, finished };

		UnitPrecondition*       		baseunit;
		ResourcesPrecondition*  		resources;
		SupplyPrecondition* 			supply;
		RequirementsPrecondition*		requirements;
		Precondition*					extra;
		StatusType 						status;
		UnitPrecondition*				postworker;
		Unit*							worker;
		int 							starttime;
		int								tries;

		UnitTrainerPrecondition(UnitPrecondition* u, ResourcesPrecondition* r, SupplyPrecondition* s, RequirementsPrecondition* req, 
								const UnitType& ut, Precondition* e)
			: UnitPrecondition(1, ut, Position(u->pos)), baseunit(u), resources(r), supply(s), requirements(req), extra(e), status(pending), 
			  postworker(NULL), worker(NULL), starttime(0), tries(0)
		{
			updateTime();
			postworker = new UnitPrecondition(Precondition::Impossible, baseunit->ut, baseunit->pos);
		}
		
		~UnitTrainerPrecondition()
		{
			Containers::remove(list, this);

			release(baseunit);
			release(resources);
			release(supply);
			release(requirements);
			release(extra);
		}

		bool updateTime()
		{
			switch (status)
			{
				case pending:
					if (updateTimePreconditions(this, ut.buildTime(), baseunit, resources, supply, requirements, extra)) {
						start();
						time = Broodwar->getFrameCount() + ut.buildTime();
						LOG << "training " << ut.getName() << " started.";
					}
					break;
				
				case tryagain:
					time = Broodwar->getFrameCount() + ut.buildTime();
					if (hasStarted()) {
						freeResources();
						status = waiting;
						LOG << "waiting for trained unit '" << ut.getName() << "' to finish.";
					} else {
						start();
					}
					break;

				case commanded:
					time = Broodwar->getFrameCount() + ut.buildTime();
					if (hasStarted()) {
						freeResources();
						status = waiting;
						LOG << "waiting for trained unit '" << ut.getName() << "' to finish.";
					} else if (Broodwar->getFrameCount() > starttime + savetime) {
						start();
						LOG << "unit " << ut.getName() << " restarted (try " << tries << ").";
					}
					break;

				case waiting:
					if (isFinished()) {
						postworker->time = 0;
						postworker->unit = worker;
						time   = 0;
						status = finished;
						LOG << "unit " << ut.getName() << " finished.";
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
				worker = baseunit->unit;
				if (ut.getRace() != Races::Zerg) {
					postworker->unit = worker;
				} else if (ut != UnitTypes::Zerg_Extractor) {
					unit = worker;
				} else {
					// Do nothing, worker will not survive.
				}
				release(baseunit);
			}
			assert(worker != NULL);
			//LOG << "Sending worker to build " << ut.getName();
			if (!worker->train(ut)) {
				auto err = Broodwar->getLastError();
				LOG << "Error: Unable to train unit '" << ut.getName() << "': " << err.toString();
				if (   (err == Errors::Insufficient_Minerals) 
					|| (err == Errors::Insufficient_Gas)
					|| (err == Errors::Insufficient_Supply))
				{
					status = pending;
					return;
				}
			}
			status = commanded;
			++tries;
			starttime = Broodwar->getFrameCount();
		}
		
		bool hasStarted() const
		{
			return (unit != NULL);
		}
		
		bool isFinished() const
		{
			return !worker->isTraining();
		}

		void freeResources()
		{
			release(resources);
			release(supply);
			release(requirements);
			release(extra);
		}
		
		bool near(const TilePosition& p1, const TilePosition& p2) const
		{
			return (std::abs(p1.x() - p2.x()) <= ut.tileWidth()) && (std::abs(p1.y() - p2.y()) <= ut.tileHeight());
		}
		
		bool onAssignUnit(Unit* u)
		{
			if ((status != commanded) && (status != tryagain))
				return false;
			if (u->getType() != ut)
				return false;
			//if (!near(u->getTilePosition(), pos->pos))
			//	return false;
			unit = u;
			return true;
		}
		
		/*
		const char* getStatusText() const
		{
			switch (status)
			{
				case pending:
					return "pending";
				case tryagain:
					return "tryagain";
				case commanded:
					return "commanded";
				case waiting:
					return "waiting";
				case finished:
				default:
					return "finished";
			}
		}
		
		void onDrawPlan() const
		{
			int x, y, width = 32*ut.tileWidth(), height = 32*ut.tileHeight();
			if (pos != NULL) {
				Position p(pos->pos);
				x = p.x();
				y = p.y();
			} else {
				Position p = unit->getPosition();
				x = p.x() - width/2;
				y = p.y() - height/2;
			}
			
			Broodwar->drawBoxMap(x, y, x + width, y + height, Colors::Green, false);
			Broodwar->drawTextMap(x+2, y+2,  "%s", ut.getName().c_str());
			Broodwar->drawTextMap(x+2, y+18, "%s", getStatusText());
			Broodwar->drawTextMap(x+2, y+34, "at %d", time);
			Broodwar->drawTextMap(x+2, y+50, "wish %d", wishtime);
		}
		*/
	};
}

std::pair<UnitPrecondition*, UnitPrecondition*> trainUnit(UnitPrecondition* worker, ResourcesPrecondition* res, SupplyPrecondition* supply, const BWAPI::UnitType& ut, Precondition* extra)
{
	RequirementsPrecondition* req = getRequirements(ut);
	// req maybe NULL.
	
	UnitTrainerPrecondition* result = new UnitTrainerPrecondition(worker, res, supply, req, ut, extra);
    list.push_back(result);
	
	UnitPrecondition* first  = result;
	UnitPrecondition* second = result->postworker;
    return std::make_pair(first, second);
}

std::pair<UnitPrecondition*, UnitPrecondition*> trainUnit(UnitPrecondition* worker, ResourcesPrecondition* res, const BWAPI::UnitType& ut, Precondition* extra)
{
	SupplyPrecondition* supply = getSupply(ut);
	// supply maybe NULL.
	return trainUnit(worker, res, supply, ut, extra);
}

std::pair<UnitPrecondition*, UnitPrecondition*> trainUnit(ResourcesPrecondition* res, const BWAPI::UnitType& ut, Precondition* extra)
{
	UnitPrecondition* worker = getIdleUnit(ut.whatBuilds().first);
	if (worker == NULL)
		return std::pair<UnitPrecondition*, UnitPrecondition*>(NULL, NULL);
	return trainUnit(worker, res, ut, extra);
}

std::pair<UnitPrecondition*, UnitPrecondition*> trainUnit(const BWAPI::UnitType& ut, Precondition* extra)
{
	ResourcesPrecondition* res = getResources(ut);
	if (res == NULL)
		return std::pair<UnitPrecondition*, UnitPrecondition*>(NULL, NULL);
	return trainUnit(res, ut, extra);
}

void trainUnitEx(const BWAPI::UnitType& ut)
{
	rememberIdle(rememberSecond(trainUnit(ut)));
}

void trainWorker(const BWAPI::UnitType& ut)
{
	useWorker(rememberSecond(trainUnit(ut)));
}

void UnitTrainerCode::onMatchEnd()
{
	list.clear();
}

void UnitTrainerCode::onTick()
{
	Containers::remove_if(list, std::mem_fun(&UnitTrainerPrecondition::updateTime));
}

bool UnitTrainerCode::onAssignUnit(BWAPI::Unit* unit)
{
	for (auto it : list)
		if (it->onAssignUnit(unit))
			return true;
	return false;
}

void UnitTrainerCode::onDrawPlan()
{
	//for (auto it : list)
	//	it->onDrawPlan();
}

void UnitTrainerCode::onCheckMemoryLeaks()
{
	UnitTrainerPrecondition::checkObjectsAlive();
}