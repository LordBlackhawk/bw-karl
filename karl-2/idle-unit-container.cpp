// ToDo:
//  * 

#include "idle-unit-container.hpp"
#include "code-list.hpp"
#include "mineral-line.hpp"
#include "vector-helper.hpp"
#include "debugger.hpp"
#include "utils/debug.h"
#include <algorithm>
#include <functional>
#include <set>
#include <vector>

using namespace BWAPI;
using namespace std::placeholders;

namespace
{
	const int larva_span_time = 300;

	struct LarvaPlaner;
	
	struct LarvaPrecondition : public UnitPrecondition
	{
		LarvaPlaner* planer;
		
		LarvaPrecondition()
			: UnitPrecondition(), planer(NULL)
		{ }
		
		~LarvaPrecondition();
		
		int sortTime() const
		{
			if (time == 0) {
				if (wishtime > Broodwar->getFrameCount() + 10)
					return wishtime;
				return time;
			} else {
				return std::max(time, wishtime);
			}
		}
	};
	
	struct LarvaSorter
	{
		bool operator () (LarvaPrecondition* lhs, LarvaPrecondition* rhs)
		{
			return lhs->sortTime() < rhs->sortTime();
		}
	};

	bool isNotAssigned(UnitPrecondition* pre)
	{
		return pre->time != 0;
	}
	
	struct LarvaPlaner
	{
		UnitPrecondition* 				prehatch;
		BWAPI::Unit* 					hatch;
		std::set<Unit*>					idlelarvas;
		std::vector<LarvaPrecondition*>	reserved;
		int								nextlarvatime;
		
		LarvaPlaner(Unit* u)
			: prehatch(NULL), hatch(u)
		{
			idlelarvas    = hatch->getLarva();
			nextlarvatime = (idlelarvas.empty()) ? hatch->getRemainingTrainTime() : 0;
		}
		
		LarvaPlaner(UnitPrecondition* u)
			: prehatch(u), hatch(NULL)
		{
			nextlarvatime = prehatch->time;
		}
		
		void distributeLarva(Unit* unit)
		{
			auto it = std::find_if(reserved.begin(), reserved.end(), isNotAssigned);
			if (it == reserved.end()) {
				idlelarvas.insert(unit);
				return;
			}
			
			(*it)->time = 0;
			(*it)->unit = unit;
		}
		
		UnitPrecondition* reserveLarva()
		{
			LarvaPrecondition* result = new LarvaPrecondition();
			result->ut = UnitTypes::Zerg_Larva;
			moveLarva(result);
			return result;
		}
		
		void moveLarva(LarvaPrecondition* pre)
		{
			reserved.push_back(pre);
			pre->planer = this;
			
			if (!idlelarvas.empty()) {
				Unit* unit = *idlelarvas.begin();
				idlelarvas.erase(unit);
				if (idlelarvas.empty()) {
					int dt = hatch->getRemainingTrainTime();
					if (dt == 0)
						dt = larva_span_time;
					nextlarvatime = Broodwar->getFrameCount() + dt;
				}
				
				pre->time = 0;
				pre->unit = unit;
				pre->pos  = unit->getPosition();
				return;
			}
			
			pre->time = nextlarvatime;
			pre->pos  = (prehatch != NULL) ? prehatch->pos : hatch->getPosition();
			pre->unit = NULL;
			nextlarvatime += larva_span_time;
		}
		
		void update()
		{
			std::stable_sort(reserved.begin(), reserved.end(), LarvaSorter());
			
			for (auto it : reserved)
				if (it->unit != NULL) {
					idlelarvas.insert(it->unit);
					it->unit = NULL;
				}
			
			int dt = hatch->getRemainingTrainTime();
			if (dt == 0)
				dt = larva_span_time;
			nextlarvatime = Broodwar->getFrameCount() + dt;
			
			for (auto it : reserved) {
				if (!idlelarvas.empty()) {
					it->time = 0;
					it->unit = *idlelarvas.begin();
					idlelarvas.erase(it->unit);
				} else {
					it->time = nextlarvatime;
					it->unit = NULL;
					nextlarvatime += larva_span_time;
				}
			}
		}
		
		void onDebug()
		{
			LOG << "Hatchery larva list:";
			for (auto it : reserved)
				LOG << "\t" << debugName(it) << " at " << it->time << " (" << it->wishtime << ") ";
		}
	};
	
	LarvaPrecondition::~LarvaPrecondition()
	{
		VectorHelper::remove(planer->reserved, this);
	}
	
	struct MinTimeSorter
	{
		bool operator () (LarvaPlaner* lhs, LarvaPlaner* rhs)
		{
			return lhs->nextlarvatime < rhs->nextlarvatime;
		}
	};
	
	std::set<Unit*> 				idleunits;
	std::set<LarvaPlaner*>			reservedlarvas;
	
	void createHatch(Unit* u)
	{
		reservedlarvas.insert(new LarvaPlaner(u));
	}
	
	UnitPrecondition* reserveLarva()
	{
		if (reservedlarvas.empty()) {
			return NULL;
		}
		
		auto it = std::min_element(reservedlarvas.begin(), reservedlarvas.end(), MinTimeSorter());
		return (*it)->reserveLarva();
	}
	
	void moveLarva(LarvaPrecondition* unit)
	{
		if (reservedlarvas.empty()) {
			unit->time = -1;
			return;
		}
		
		auto it = std::min_element(reservedlarvas.begin(), reservedlarvas.end(), MinTimeSorter());
		(*it)->moveLarva(unit);
	}
	
	bool isHatchery(LarvaPlaner* p, Unit* hatch)
	{
		return p->hatch == hatch;
	}
	
	void distributeLarva(Unit* unit)
	{
		if (Broodwar->getFrameCount() == 0)
			return;

		Unit* hatch = unit->getHatchery();
		auto it = std::find_if(reservedlarvas.begin(), reservedlarvas.end(), std::bind(isHatchery, _1, hatch));
		if (it == reservedlarvas.end()) {
			LOG << "Error: Hatchery of larva not found in list.";
			return;
		}
		(*it)->distributeLarva(unit);
	}
	
	void removeHatchery(Unit* unit)
	{
		auto it = std::find_if(reservedlarvas.begin(), reservedlarvas.end(), std::bind(isHatchery, _1, unit));
		if (it == reservedlarvas.end()) {
			LOG << "Error: Destroyed hatchery not found in list.";
			return;
		}
		
		LarvaPlaner* old = *it;
		reservedlarvas.erase(it);
		for (auto it : old->reserved)
			moveLarva(it);
		free(old);
	}

	UnitPrecondition* createAndErase(Unit* unit)
	{
		idleunits.erase(unit);
		return new UnitPrecondition(unit);
	}
}

UnitPrecondition* getIdleUnit(const BWAPI::UnitType& ut)
{
	for (auto it : idleunits)
		if (it->getType() == ut)
			return createAndErase(it);
	if (ut == UnitTypes::Zerg_Larva)
		return reserveLarva();
	return NULL;
}

void IdleUnitContainerCode::onMatchBegin()
{
	for (auto it : Broodwar->self()->getUnits())
		if (it->getType() == UnitTypes::Zerg_Hatchery)
			createHatch(it);
}

void IdleUnitContainerCode::onMatchEnd()
{
	idleunits.clear();
	VectorHelper::clear_and_delete(reservedlarvas);
}

void IdleUnitContainerCode::onTick()
{
	for (auto it : reservedlarvas)
		it->update();
}

void IdleUnitContainerCode::onUnitCreate(BWAPI::Unit* unit)
{
	if (unit->getPlayer() != Broodwar->self())
		return;
		
	if (CodeList::onAssignUnit(unit))
		return;

	UnitType ut = unit->getType();
	if (ut == UnitTypes::Zerg_Larva) {
		distributeLarva(unit);
		return;
	}
	
	if (ut.isWorker()) {
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
	
	UnitType ut = unit->getType();
	if (   (ut == UnitTypes::Zerg_Hatchery)
	    || (ut == UnitTypes::Zerg_Lair)
		|| (ut == UnitTypes::Zerg_Hive))
	{
		removeHatchery(unit);
	}
}

void IdleUnitContainerCode::onDebug()
{
	for (auto it : reservedlarvas)
		it->onDebug();
}
