#include "larvas.hpp"
#include "vector-helper.hpp"
#include "precondition-helper.hpp"
#include "utils/debug.h"
#include <algorithm>
#include <functional>
#include <set>

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
		UnitPrecondition* 				posthatch;
		BWAPI::Unit* 					hatch;
		std::set<Unit*>					idlelarvas;
		std::vector<LarvaPrecondition*>	reserved;
		int								nextlarvatime;
		
		LarvaPlaner(Unit* u)
			: prehatch(NULL), posthatch(NULL), hatch(u)
		{
			idlelarvas    = hatch->getLarva();
			nextlarvatime = (idlelarvas.empty()) ? hatch->getRemainingTrainTime() : 0;
		}
		
		LarvaPlaner(UnitPrecondition* u)
			: prehatch(u), hatch(NULL)
		{
			nextlarvatime = prehatch->time;
			posthatch = new UnitPrecondition(prehatch->time, prehatch->ut, prehatch->pos);
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
					int dt = (prehatch != NULL) ? larva_span_time : hatch->getRemainingTrainTime();
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
			if (prehatch != NULL) {
				posthatch->time = prehatch->time;
				prehatch->wishtime = posthatch->wishtime;
				if (prehatch->time == 0) {
					hatch = prehatch->unit;
					posthatch->unit = hatch;
					release(prehatch);
					posthatch = NULL;
					idlelarvas = hatch->getLarva();
				}
			}
		
			std::stable_sort(reserved.begin(), reserved.end(), LarvaSorter());
			
			for (auto it : reserved)
				if (it->unit != NULL) {
					idlelarvas.insert(it->unit);
					it->unit = NULL;
				}
			
			int dt = (prehatch != NULL) ? larva_span_time : hatch->getRemainingTrainTime();
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
		
		void onDrawPlan()
		{
			int x, y;
			if (prehatch != NULL) {
				x = prehatch->pos.x();
				y = prehatch->pos.y();
			} else {
				Position pos = hatch->getPosition();
				x = pos.x();
				y = pos.y();
			}
			
			int assigned = idlelarvas.size();
			for (auto it : reserved)
				if (it->time == 0)
					++assigned;

			Broodwar->drawTextMap(x, y, "%d / %d", assigned, reserved.size());
		}
		
		/*
		void onDebug()
		{
			LOG << "Hatchery larva list:";
			for (auto it : reserved)
				LOG << "\t" << debugName(it) << " at " << it->time << " (" << it->wishtime << ") ";
		}
		*/
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
	
	std::set<LarvaPlaner*>			reservedlarvas;
	
	void createHatch(Unit* u)
	{
		reservedlarvas.insert(new LarvaPlaner(u));
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
		release(old);
	}
}

UnitPrecondition* getLarva()
{
	if (reservedlarvas.empty()) {
		return NULL;
	}
	
	auto it = std::min_element(reservedlarvas.begin(), reservedlarvas.end(), MinTimeSorter());
	return (*it)->reserveLarva();
}

UnitPrecondition* registerHatchery(UnitPrecondition* hatch)
{
	LarvaPlaner* planer = new LarvaPlaner(hatch);
	reservedlarvas.insert(planer);
	return planer->posthatch;
}

void LarvaCode::onMatchBegin()
{
	for (auto it : Broodwar->self()->getUnits())
		if (it->getType() == UnitTypes::Zerg_Hatchery)
			createHatch(it);
}

void LarvaCode::onMatchEnd()
{
	VectorHelper::clear_and_delete(reservedlarvas);
}

void LarvaCode::onTick()
{
	for (auto it : reservedlarvas)
		it->update();
}

bool LarvaCode::onAssignUnit(BWAPI::Unit* unit)
{
	if (unit->getType() == UnitTypes::Zerg_Larva) {
		distributeLarva(unit);
		return true;
	}
	return false;
}

void LarvaCode::onUnitDestroy(BWAPI::Unit* unit)
{
	Player* self = Broodwar->self();
	if (unit->getPlayer() != self)
		return;
	
	UnitType ut = unit->getType();
	if (   (ut == UnitTypes::Zerg_Hatchery)
	    || (ut == UnitTypes::Zerg_Lair)
		|| (ut == UnitTypes::Zerg_Hive))
	{
		removeHatchery(unit);
	}
}

void LarvaCode::onDrawPlan()
{
	for (auto it : reservedlarvas)
		it->onDrawPlan();
}

/*
void LarvaCode::onDebug()
{
	for (auto it : reservedlarvas)
		it->onDebug();
}
*/
