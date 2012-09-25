// ToDo:
//  * estimatedProduction improving by looking into future.
//  * adding routines for harvesting gas.
//  * better implementation of getWorker.

#include "mineral-line.hpp"
#include "container-helper.hpp"
#include "object-counter.hpp"
#include "building-placer.hpp"
#include "unit-builder.hpp"
#include "precondition-helper.hpp"
#include "log.hpp"
#include "bwapi-helper.hpp"
#include "hungarian-algorithm.hpp"
#include <BWAPI.h>
#include <BWTA.h>
#include <algorithm>
#include <functional>

using namespace BWAPI;

#define THIS_DEBUG DEBUG

std::vector<Production> estimatedProduction;

namespace
{
	#include "mineral-line-impl.hpp"

	template <class Derived>
	struct WorkerLine
	{
		UnitPrecondition*	pre;
		Unit*				base;
		std::set<Unit*> 	worker;
		
		WorkerLine(UnitPrecondition* p)
			: pre(p), base(NULL)
		{ }
		
		WorkerLine(Unit* u)
			: pre(NULL), base(u)
		{ }
		
		Derived* This()
		{
			return static_cast<Derived*>(this);
		}
		
		void sendWorker(Unit* w)
		{
			if (!w->rightClick(This()->getTarget(w)))
				LOG << "Sending worker, but recieved error: " << Broodwar->getLastError().toString();
		}
	
		void addWorker(Unit* w)
		{
			worker.insert(w);
			sendWorker(w);
		}
		
		void onOwnUnitDestroy(Unit* u)
		{
			worker.erase(u);
		}
		
		int estimateProduction() const
		{
			return 45 * worker.size();
		}
		
		void onActivated()
		{ }
		
		bool update()
		{
			if (pre != NULL) {
				if (!pre->isFulfilled())
					return false;
				
				base = pre->unit;
				release(pre);
				This()->onActivated();
				return true;
			}
			return false;
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
		
		Unit* getNearestWorker(const Position& pos)
		{
			Unit* result = getNearest(worker, pos);
			worker.erase(result);
			return result;
		}
	};
	
	struct GasLine;
	
	struct MineralLine : public WorkerLine<MineralLine>, public ObjectCounter<MineralLine>
	{
		BWTA::BaseLocation*		location;
		std::set<GasLine*>		gaslines;
		
		MineralLine(UnitPrecondition* p, BWTA::BaseLocation* l)
			: WorkerLine<MineralLine>(p), location(l)
		{ }
		
		MineralLine(Unit* b, BWTA::BaseLocation* l)
			: WorkerLine<MineralLine>(b), location(l)
		{ }
		
		Unit* getTarget(Unit* w) const
		{
			return getNearest(location->getMinerals(), w->getPosition());
		}
		
		bool updateActive()
		{
			return !base->exists();
		}
		
		Unit* getUnusedGeyser() const;
		bool isYourGeyser(const Position& pos) const;
	};
	
	struct GasLine : public WorkerLine<GasLine>, public ObjectCounter<GasLine>
	{
		MineralLine*		mineralline;
		int 				init_count;
	
		GasLine(UnitPrecondition* p, MineralLine* b, int c)
			: WorkerLine<GasLine>(p), mineralline(b), init_count(c)
		{
			mineralline->gaslines.insert(this);
			
			for (auto it : Broodwar->getStaticGeysers())
				if (it->getTilePosition() == pre->pos) {
					base = it;
					break;
				}

			if (base == NULL)
				LOG << "No Geyser found!";
		}
		
		~GasLine()
		{
			mineralline->gaslines.erase(this);
		}
		
		Unit* getTarget(Unit* /*w*/) const
		{
			return base;
		}
		
		bool updateActive()
		{
			return !base->getType().isRefinery();
		}
		
		void onActivated()
		{
			LOG << "Refinery build!";
			for (int k=0; k<init_count; ++k)
				incWorker();
		}
		
		void incWorker()
		{
			Unit* w = mineralline->getNearestWorker(base->getPosition());
			if (w != NULL)
				addWorker(w);
			else
				WARNING << "No nearest worker!";
		}
	};
	
	Unit* MineralLine::getUnusedGeyser() const
	{
		std::set<Unit*> geysers = location->getGeysers();
		for (auto it : gaslines)
			geysers.erase(it->base);
		if (geysers.empty())
			return NULL;
		return *geysers.begin();
	}
	
	bool MineralLine::isYourGeyser(const Position& pos) const
	{
		for (auto it : location->getGeysers()) {
			if (pos.getDistance(it->getPosition()) < 72.0)
				return true;
		}
		return false;
	}
	
	std::vector<MineralLine*>		minerallines;
	std::vector<GasLine*>			gaslines;
	
	void addWorkerNearestMineralLine(BWAPI::Unit* unit)
	{
		if (minerallines.empty()) {
			LOG << "minerallines are empty!";
			return;
		}

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
			THIS_DEBUG << "Sending worker to minerals.";
			addWorkerNearestMineralLine(unit->unit);
			release(unit);
			return true;
		}
		return false;
	}
	
	struct PlanItem
	{
		enum { tMineralLine, tGasLine, tIncomingWorker, tOutgoingWorker };
		int type;
		union {
			MineralLine*		mineralline;
			GasLine*			gasline;
			UnitPrecondition*	iworker;
			UnitPrecondition*	oworker;
		};
		
		PlanItem(MineralLine* m)
			: type(tMineralLine), mineralline(m)
		{ }
		
		PlanItem(GasLine* g)
			: type(tGasLine), gasline(g)
		{ }
		
		PlanItem(int t, UnitPrecondition* pre)
			: type(t), iworker(pre)
		{ }
		
		void release()
		{
			switch (type)
			{
				case tMineralLine:
					delete mineralline;
					break;
				case tGasLine:
					delete gasline;
					break;
				case tIncomingWorker:
					::release(iworker);
					break;
				case tOutgoingWorker:
					::release(oworker);
					break;
			}
		}
		
		bool update()
		{
			switch (type)
			{
				case tMineralLine:
					return mineralline->update();
				case tGasLine:
					return gasline->update();
				case tIncomingWorker:
					return checkWorkerReady(iworker);
				case tOutgoingWorker:
					// TODO!!!
					return false;
				default:
					return false;
			}
		}
		
		int getTime() const
		{
			switch (type)
			{
				case tMineralLine:
					return mineralline->pre->time;
				case tGasLine:
					return gasline->pre->time;
				case tIncomingWorker:
					return iworker->time;
				case tOutgoingWorker:
					return oworker->time;
				default:
					return Precondition::Impossible;
			}
		}
		
		bool operator < (const PlanItem& other) const
		{
			return (getTime() < other.getTime());
		}
		
		Production estimateProduction(const Production& last) const
		{
			Production result;
			result.time     = getTime();
			result.minerals = last.minerals;
			result.gas      = last.gas;
			switch (type)
			{
				case tMineralLine:
					break;
				case tGasLine:
					result.minerals -= 45 * gasline->init_count;
					result.gas      += 45 * gasline->init_count;
					break;
				case tIncomingWorker:
					result.minerals += 45;
					break;
				case tOutgoingWorker:
					result.minerals -= 45;
					break;
			}
			return result;
		}
	};

	std::vector<PlanItem>			plan;
	
	int sumEstimatedProduction()
	{
		int sum = 0;
		for (auto it : minerallines)
			sum += it->estimateProduction();
		return sum;
	}
	
	int sumEstimatedGasProduction()
	{
		int sum = 0;
		for (auto it : gaslines)
			sum += it->estimateProduction();
		return sum;
	}
}

void useWorker(BWAPI::Unit* unit)
{
	addWorkerNearestMineralLine(unit);
}

void useWorker(UnitPrecondition* unit)
{
	if (unit == NULL) {
		WARNING << "called useWorker with unit == NULL.";
		return;
	}

	if (unit->isFulfilled()) {
		THIS_DEBUG << "Sending worker immediately.";
		useWorker(unit->unit);
		release(unit);
		return;
	}
	
	plan.push_back(PlanItem(PlanItem::tIncomingWorker, unit));
}

UnitPrecondition* getWorker(const BWAPI::Race& r)
{
	for (auto it : minerallines) {
		UnitPrecondition* result = it->getWorker(r);
		if (result == NULL)
			continue;
		
		plan.push_back(PlanItem(PlanItem::tOutgoingWorker, result));
		return result;
	}
	
	/*
	for (auto it : plan) {
		UnitPrecondition* result = it->getWorker(r);
		if (result == NULL)
			continue;
		
		plan.push_back(PlanItem(PlanItem::tOutgoingWorker, result));
		return result;
	}
	*/
	
	return NULL;
}

UnitPrecondition* registerBase(UnitPrecondition* b)
{
	// TODO !!!
	return b;
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
		plan.push_back(PlanItem(gas));
		return;
	}
	
	WARNING << "No mineral line for refinery found!";
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
		LOG << "adding starting depot!";
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
	Containers::clear_and_delete(minerallines);
	Containers::clear_and_delete(gaslines);

	for (auto it : plan)
		it.release();
	plan.clear();
}

void MineralLineCode::onTick()
{
	Containers::remove_if(plan,         std::mem_fun_ref(&PlanItem::update));
	Containers::remove_if(minerallines, std::mem_fun(&MineralLine::updateActive));
	Containers::remove_if(gaslines,     std::mem_fun(&GasLine::updateActive));
	
	std::sort(plan.begin(), plan.end());
	
	Production& prod = estimatedProduction[0];
	prod.minerals = sumEstimatedProduction();
	prod.gas      = sumEstimatedGasProduction();
	
	int index = 1;
	estimatedProduction.resize(1 + plan.size());
	for (auto it : plan) {
		estimatedProduction[index] = it.estimateProduction(estimatedProduction[index-1]);
		++index;
	}
	
	for (auto& it : estimatedProduction) {
		it.minerals = std::max(1, it.minerals);
		it.gas      = std::max(1, it.gas);
	}
}

void MineralLineCode::onUnitDestroy(BWAPI::Unit* unit)
{
	for (auto it : minerallines)
		it->onOwnUnitDestroy(unit);
	for (auto it : gaslines)
		it->onOwnUnitDestroy(unit);
}

void MineralLineCode::onCheckMemoryLeak()
{
	MineralLine::checkObjectsAlive();
	GasLine::checkObjectsAlive();
}
