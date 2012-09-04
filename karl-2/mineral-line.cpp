// ToDo:
//  * estimatedProduction improving by looking into future.
//  * adding routines for harvesting gas.

#include "mineral-line.hpp"
#include "vector-helper.hpp"
#include "utils/debug.h"
#include <BWAPI.h>
#include <BWTA.h>
#include <algorithm>

using namespace BWAPI;

std::vector<Production> estimatedProduction;

namespace
{
	BWAPI::Unit* getNearest(std::set<BWAPI::Unit*> units, BWAPI::Position p)
	{
		if (units.empty())
			return NULL;
		
		auto it = units.begin();
		auto itend = units.end();
		BWAPI::Unit* result = *it;
		double dis = p.getDistance((*it)->getPosition());
		++it;
		
		while (it != itend)
		{
			double newdis = p.getDistance((*it)->getPosition());
			if (newdis < dis) {
				result = *it;
				dis    = newdis;
			}
			++it;
		}
		
		return result;
	}

	struct MineralLine
	{
		BWTA::BaseLocation*		location;
		std::set<BWAPI::Unit*>	worker;
		
		MineralLine(BWTA::BaseLocation* l)
			: location(l)
		{ }

		void addWorker(BWAPI::Unit* w)
		{
			worker.insert(w);
			sendWorker(w);
		}
		
		void sendWorker(BWAPI::Unit* w)
		{
			BWAPI::Unit* m = getNearest(location->getMinerals(), w->getPosition());
			w->rightClick(m);
		}
		
		void onUnitDestroy(BWAPI::Unit* u)
		{
			worker.erase(u);
		}
		
		int estimateProduction() const
		{
			return 45 * worker.size();
		}
	};

	std::set<MineralLine*>			minerallines;
	std::vector<UnitPrecondition*>	newworker;
	
	void addWorkerNearestMineralLine(BWAPI::Unit* unit)
	{
		if (minerallines.empty())
			return;

		Position pos      = unit->getPosition();
		auto it           = minerallines.begin();
		auto itend        = minerallines.end();
		MineralLine* best = *it;
		double bestdis    = pos.getDistance((*it)->location->getPosition());
		
		for (++it; it!=itend; ++it) {
			double dis = pos.getDistance((*it)->location->getPosition());
			if (dis < bestdis) {
				best    = *it;
				bestdis = dis;
			}
		}
		
		best->addWorker(unit);
	}
	
	bool checkWorkerReady(UnitPrecondition* unit)
	{
		if (unit->time == 0) {
			LOG << "Sending worker to minerals.";
			addWorkerNearestMineralLine(unit->unit);
			return true;
		}
		return false;
	}
	
	int sumEstimatedProduction()
	{
		int sum = 0;
		for (auto it : minerallines)
			sum += it->estimateProduction();
		return sum;
	}
}

void useWorker(UnitPrecondition* unit)
{
	if (unit->time == 0) {
		LOG << "Sending worker immediately.";
		addWorkerNearestMineralLine(unit->unit);
		return;
	}
	
	newworker.push_back(unit);
}

void MineralLineCode::onMatchBegin()
{
	BWAPI::Player* self = BWAPI::Broodwar->self();
	MineralLine* obj    = new MineralLine(BWTA::getStartLocation(self));
	for (auto it : self->getUnits())
		if (it->getType().isWorker())
			obj->addWorker(it);
	minerallines.insert(obj);
	
	estimatedProduction.resize(1);
	Production& prod = estimatedProduction[0];
	prod.time     = 0;
	prod.minerals = 4*45;
	prod.gas      = 1;
}

void MineralLineCode::onMatchEnd()
{
	for (auto it : minerallines)
		delete it;
	minerallines.clear();
	newworker.clear();
}

void MineralLineCode::onTick()
{
	VectorHelper::remove_if(newworker, checkWorkerReady);
	
	Production& prod = estimatedProduction[0];
	prod.minerals = sumEstimatedProduction();
}

void MineralLineCode::onUnitDestroy(BWAPI::Unit* unit)
{
	for (auto it : minerallines)
		it->onUnitDestroy(unit);
}
