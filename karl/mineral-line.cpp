// ToDo:
//  * estimatedProduction improving by looking into future.
//  * adding routines for harvesting gas.
//  * better implementation of getWorker.

#include "mineral-line.hpp"
#include "vector-helper.hpp"
#include "object-counter.hpp"
#include "building-placer.hpp"
#include "unit-builder.hpp"
#include "precondition-helper.hpp"
#include "utils/debug.h"
#include <BWAPI.h>
#include <BWTA.h>
#include <algorithm>
#include <functional>

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
	
	template <class Derived>
	struct WorkerLine
	{
		std::set<Unit*> 	worker;
		
		Derived* This()
		{
			return static_cast<Derived*>(this);
		}
		
		void sendWorker(Unit* w)
		{
			w->rightClick(This()->getTarget(w));
		}
	
		void addWorker(Unit* w)
		{
			worker.insert(w);
			sendWorker(w);
		}
		
		void onUnitDestroy(Unit* u)
		{
			worker.erase(u);
		}
		
		int estimateProduction() const
		{
			return 45 * worker.size();
		}
		
		UnitPrecondition* getWorker(const Race& r)
		{
			for (auto it : worker)
				if (it->getType().getRace() == r) {
					worker.erase(it);
					return new UnitPrecondition(it);
				}
			return NULL;
		}
	};
	
	struct GasLine;
	
	struct MineralLine : public WorkerLine<MineralLine>, public ObjectCounter<MineralLine>
	{
		UnitPrecondition*		pre;
		Unit*					base;
		BWTA::BaseLocation*		location;
		std::set<GasLine*>		gaslines;
		
		MineralLine(Unit* b, BWTA::BaseLocation* l)
			: pre(NULL), base(b), location(l)
		{ }
		
		MineralLine(UnitPrecondition* p, BWTA::BaseLocation* l)
			: pre(p), base(NULL), location(l)
		{ }
		
		Unit* getTarget(Unit* w) const
		{
			return getNearest(location->getMinerals(), w->getPosition());
		}
		
		bool update()
		{
			if (pre != NULL) {
				if (!pre->isFulfilled())
					return false;
				
				base = pre->unit;
				release(pre);
			}
			
			return !base->exists();
		}
		
		Unit* getNearestWorker(const Position& pos)
		{
			Unit* result = getNearest(worker, pos);
			worker.erase(result);
			return result;
		}
		
		Unit* getUnusedGeyser() const;
		bool isYourGeyser(const Position& pos) const;
	};
	
	struct GasLine : public WorkerLine<GasLine>, public ObjectCounter<GasLine>
	{
		UnitPrecondition* 	pre;
		Unit*				refinery;
		MineralLine*		base;
		int 				init_count;
	
		GasLine(UnitPrecondition* p, MineralLine* b, int c)
			: pre(p), refinery(NULL), base(b), init_count(c)
		{
			base->gaslines.insert(this);
			
			for (auto it : Broodwar->getStaticGeysers())
				if (it->getTilePosition() == pre->pos) {
					refinery = it;
					break;
				}
			
			if (refinery == NULL)
				LOG << "No Geyser found!";
		}
		
		~GasLine()
		{
			base->gaslines.erase(this);
		}
		
		Unit* getTarget(Unit* /*w*/) const
		{
			return refinery;
		}
		
		bool update()
		{
			if (pre != NULL) {
				if (!pre->isFulfilled())
					return false;
				
				refinery = pre->unit;
				release(pre);
				
				LOG << "Refinery build!";
				for (int k=0; k<init_count; ++k)
					incWorker();
			}

			return (refinery->getType() == UnitTypes::Resource_Vespene_Geyser);
		}
		
		void incWorker()
		{
			Unit* w = base->getNearestWorker(refinery->getPosition());
			if (w != NULL)
				addWorker(w);
			else
				LOG << "No nearest worker!";
		}
	};
	
	Unit* MineralLine::getUnusedGeyser() const
	{
		std::set<Unit*> geysers = location->getGeysers();
		for (auto it : gaslines)
			geysers.erase(it->refinery);
		if (geysers.empty())
			return NULL;
		return *geysers.begin();
	}
	
	bool MineralLine::isYourGeyser(const Position& pos) const
	{
		for (auto it : location->getGeysers()) {
			LOG << "pos: " << pos.x() << "," << pos.y() << " =?= " << it->getPosition().x() << "," << it->getPosition().y();
			if (pos.getDistance(it->getPosition()) < 72.0)
				return true;
		}
		return false;
	}

	std::vector<MineralLine*>		minerallines;
	std::vector<GasLine*>			gaslines;
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
		if (unit->isFulfilled()) {
			//LOG << "Sending worker to minerals.";
			addWorkerNearestMineralLine(unit->unit);
			release(unit);
			return true;
		}
		return false;
	}
	
	int sumEstimatedProduction()
	{
		int sum = 0;
		for (auto it : minerallines)
			sum += it->estimateProduction();
		return std::max(1, sum);
	}
	
	int sumEstimatedGasProduction()
	{
		int sum = 0;
		for (auto it : gaslines)
			sum += it->estimateProduction();
		return std::max(1, sum);
	}
}

void useWorker(BWAPI::Unit* unit)
{
	addWorkerNearestMineralLine(unit);
}

void useWorker(UnitPrecondition* unit)
{
	if (unit == NULL) {
		LOG << "called useWorker with unit == NULL.";
		return;
	}

	if (unit->time == 0) {
		//LOG << "Sending worker immediately.";
		useWorker(unit->unit);
		return;
	}
	
	newworker.push_back(unit);
}

UnitPrecondition* getWorker(const BWAPI::Race& r)
{
	for (auto it : minerallines) {
		UnitPrecondition* result = it->getWorker(r);
		if (result != NULL)
			return result;
	}
	return NULL;
}

BuildingPositionPrecondition* getUnusedGeyser(const BWAPI::Race& r)
{
	for (auto it : minerallines) {
		Unit* geyser = it->getUnusedGeyser();
		if (geyser == NULL)
			continue;
		
		return getBuildingPosition(r.getRefinery(), geyser->getTilePosition());
	}
	return NULL;
}

void useRefinery(UnitPrecondition* unit, int worker)
{
	for (auto it : minerallines) {
		if (!it->isYourGeyser(unit->pos))
			continue;

		GasLine* gas = new GasLine(unit, it, worker);
		gaslines.push_back(gas);
		return;
	}
	
	LOG << "No mineral line for refinery found!";
}

void buildRefinery(const BWAPI::UnitType& type, int worker)
{
	auto pos = getUnusedGeyser(type.getRace());
	if (pos == NULL)
		return;
	
	auto result = buildUnit(pos, type);
	if (result.first == NULL)
		return;
	
	useRefinery(result.first, worker);
	if (result.second != NULL)
		useWorker(result.second);
}

void MineralLineCode::onMatchBegin()
{
	Unit* depot = NULL;
	for (auto it : Broodwar->self()->getUnits())
		if (it->getType().isResourceDepot()) {
			depot = it;
			break;
		}

	if (depot != NULL) {
		auto line = new MineralLine(depot, BWTA::getStartLocation(Broodwar->self()));
		minerallines.push_back(line);
		for (auto it : Broodwar->self()->getUnits())
			if (it->getType().isWorker())
				line->addWorker(it);
	}
	
	estimatedProduction.resize(1);
	Production& prod = estimatedProduction[0];
	prod.time     = 0;
	prod.minerals = 4*45;
	prod.gas      = 1;
}

void MineralLineCode::onMatchEnd()
{
	VectorHelper::clear_and_delete(minerallines);
	VectorHelper::clear_and_delete(gaslines);
	VectorHelper::clear_and_delete(newworker);
}

void MineralLineCode::onTick()
{
	VectorHelper::remove_if(newworker, checkWorkerReady);
	VectorHelper::remove_if(minerallines, std::mem_fun(&MineralLine::update));
	VectorHelper::remove_if(gaslines, std::mem_fun(&GasLine::update));
	
	Production& prod = estimatedProduction[0];
	prod.minerals = sumEstimatedProduction();
	prod.gas      = sumEstimatedGasProduction();
}

void MineralLineCode::onUnitDestroy(BWAPI::Unit* unit)
{
	for (auto it : minerallines)
		it->onUnitDestroy(unit);
	for (auto it : gaslines)
		it->onUnitDestroy(unit);
}

void MineralLineCode::onCheckMemoryLeak()
{
	MineralLine::checkObjectsAlive();
	GasLine::checkObjectsAlive();
}
