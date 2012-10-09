// ToDo:
//  * Bug: Hatchery forgets larvas.

#include "larvas.hpp"
#include "precondition-helper.hpp"
#include "object-counter.hpp"
#include "parallel-vector.hpp"
#include "hungarian-algorithm.hpp"
#include "unit-lifetime-observer.hpp"
#include "container-helper.hpp"
#include "valuing.hpp"
#include "utils/debug.h"
#include <algorithm>
#include <functional>
#include <set>

using namespace BWAPI;
using namespace std::placeholders;

namespace
{
	const int larva_span_time = 300;

	struct LarvaAgent;
	struct LarvaJob;
	struct LarvaPrecondition;

	struct LarvaAgent : public ObjectCounter<LarvaAgent>
	{
		// Data:
		int         time;
		Position    pos;
		Unit*       larva;
		LarvaJob*   assigned;
		bool 		remove;

		LarvaAgent();
		void markRemove();
		bool update();
	};

	struct LarvaJob : public ObjectCounter<LarvaJob>
	{
		// Data:
		int                 wishtime;
		Position            wishpos;
		LarvaPrecondition*  pre;
		LarvaAgent*         assigned;

		LarvaJob(LarvaPrecondition* p);
		void markRemove();
		bool update();
	};

	ParallelVector<LarvaAgent*>         agents;
	ParallelVector<LarvaJob*>           jobs;

	struct LarvaPrecondition : public UnitPrecondition, public ObjectCounter<LarvaPrecondition>
	{
		LarvaJob*       job;

		LarvaPrecondition()
			: UnitPrecondition(Precondition::Impossible, UnitTypes::Zerg_Larva, Positions::Unknown), job(NULL)
		{
			job = new LarvaJob(this);
		}

		~LarvaPrecondition()
		{
			job->markRemove();
		}
	};

	LarvaAgent::LarvaAgent()
		: time(Precondition::Impossible), pos(Positions::Unknown), larva(NULL), remove(false)
	{
		Containers::add(agents, this);
	}

	void LarvaAgent::markRemove()
	{
		remove = true;
	}

	bool LarvaAgent::update()
	{
		if (remove) {
			delete this;
			return true;
		}

		if (larva != NULL) {
			time = 0;
			pos  = larva->getPosition();
		}

		return false;
	}

	LarvaJob::LarvaJob(LarvaPrecondition* p)
		: wishtime(0), wishpos(Positions::Unknown), pre(p), assigned(NULL)
	{
		Containers::add(jobs, this);
	}

	void LarvaJob::markRemove()
	{
		if (assigned != NULL) {
			assigned->markRemove();
			assigned = NULL;
		}
		pre = NULL;
	}

	bool LarvaJob::update()
	{
		if (pre == NULL) {
			delete this;
			return true;
		}

		wishtime  = pre->wishtime;
		//wishpos   = pre->wishpos;
		if (assigned != NULL) {
			pre->time = assigned->time;
			pre->unit = assigned->larva;
		} else {
			pre->time = Precondition::Impossible;
			pre->unit = NULL;
		}
		return false;
	}

	struct ProblemType
	{
		double evaluate(int idAgent, int idJob) const
		{
			if (idAgent < agents.size()) {
				return 0.0;
			} else if (idJob < jobs.size()) {
				return 0.0;
			}
			
			LarvaAgent* agent = agents[idAgent];
			LarvaJob* job = jobs[idJob];
			return valueLarvaAssignment(agent->time, job->wishtime, job->assigned == agent);
		}

		void assign(int idAgent, int idJob) const
		{
			if ((idJob < 0) || (idJob >= jobs.size())) {
				agents[idAgent]->assigned = NULL;
				return;
			}
			LarvaJob* job = jobs[idJob];

			if ((idAgent < 0) || (idAgent >= agents.size())) {
				job->assigned = NULL;
				return;
			}
			LarvaAgent* agent = agents[idAgent];

			agent->assigned = job;
			job->assigned = agent;
			job->update();
		}

		int numberOfAgents() const
		{
			return agents.size();
		}

		int numberOfJobs() const
		{
			return jobs.size();
		}
	};

	struct HatcheryPlaner;

	std::vector<HatcheryPlaner*>    hatcheries;

	struct HatcheryPlaner : public UnitLifetimeObserver<HatcheryPlaner>, public ObjectCounter<HatcheryPlaner>
	{
		typedef std::vector<LarvaAgent*>		AgentContainer;
		typedef AgentContainer::iterator  		AgentIterator;

		AgentContainer    	agents;
		
		HatcheryPlaner(Unit* u)
			: UnitLifetimeObserver<HatcheryPlaner>(u)
		{
			hatcheries.push_back(this);
		}
		
		HatcheryPlaner(UnitPrecondition* p)
			: UnitLifetimeObserver<HatcheryPlaner>(p)
		{
			hatcheries.push_back(this);
		}
		
		void onRemoveFromList()
		{
			Containers::remove(hatcheries, this);
		}
		
		bool distributeLarva(Unit* u);
		void onUnitDestroyed();
		void onUpdateOnline();
		void onUpdateOffline();
		void updateRest(AgentIterator start, int time);
		void addJobs();
	};

	bool hasNoUnit(LarvaAgent* agent)
	{
		return agent->larva == NULL;
	}

	void HatcheryPlaner::onUnitDestroyed()
	{
		Containers::remove_if(agents, hasNoUnit);
	}

	bool checkAgent(LarvaAgent* agent, int* notassigned)
	{
		if (agent->remove)
			return true;

		if (agent->assigned == NULL)
			++(*notassigned);

		return false;
	}

	void HatcheryPlaner::addJobs()
	{
		int notassigned = 0;
		Containers::remove_if(agents, std::bind(checkAgent, _1, &notassigned));

		if (notassigned == 0) {
			if (agents.size() < 100)
				agents.push_back(new LarvaAgent());
			else
				LOG << "More than 100 jobs planed per hatch!";
		}
	}

	void HatcheryPlaner::onUpdateOnline()
	{
		addJobs();
		AgentIterator it    = agents.begin();
		AgentIterator itend = agents.end();
		while ((it != itend) && ((*it)->larva != NULL))
			++it;
		updateRest(it, Broodwar->getFrameCount() + larva_span_time);
	}

	void HatcheryPlaner::onUpdateOffline()
	{
		addJobs();
		updateRest(agents.begin(), pre->time);
	}

	void HatcheryPlaner::updateRest(AgentIterator start, int time)
	{
		AgentIterator end = agents.end();
		while (start != end) {
			(*start)->time  = time;
			(*start)->larva = NULL;
			++start;
			time += larva_span_time;
		}
	}

	ProblemType         				problem;
	HungarianAlgorithm<ProblemType>  	assignment(problem);
	
	bool HatcheryPlaner::distributeLarva(Unit* u)
	{
		for (auto it : agents)
			if (it->larva == NULL)
		{
			it->larva = u;
			return true;
		}
		
		LOG << "No agent for larva found, creating new!";
		LarvaAgent* agent = new LarvaAgent();
		agents.push_back(agent);
		agent->larva = u;
		return true;
	}
	
	bool distributeLarva(Unit* u)
	{
		Unit* hatch = u->getHatchery();
		for (auto planer : hatcheries) {
			if (planer->unit == hatch)
		{
			return planer->distributeLarva(u);
		}
		
		LOG << "Compared " << planer->unit << " with " << hatch << ".";
		}
		
		LOG << "Hatchery for larva not found!";
		return false;
	}
}

UnitPrecondition* getLarva()
{
	return new LarvaPrecondition();
}

UnitPrecondition* registerHatchery(UnitPrecondition* hatch)
{
	return HatcheryPlaner::createObserver(hatch);
}

void LarvaCode::onMatchBegin()
{
	for (auto it : Broodwar->self()->getUnits())
		if (it->getType() == UnitTypes::Zerg_Hatchery) {
			LOG << "Hatchery added.";
			new HatcheryPlaner(it);
		}
}

void LarvaCode::onMatchEnd()
{
	Containers::clear_and_delete(agents);
	Containers::clear_and_delete(jobs);
	Containers::clear_and_delete(hatcheries);
}

void LarvaCode::onTick()
{
	Containers::remove_if(hatcheries, std::mem_fun(&HatcheryPlaner::update));

	agents.update();
	Containers::remove_if(agents, std::mem_fun(&LarvaAgent::update));

	jobs.update();
	Containers::remove_if(jobs, std::mem_fun(&LarvaJob::update));

	assignment.execute();
}

bool LarvaCode::onAssignUnit(BWAPI::Unit* unit)
{
	return (unit->getType() == UnitTypes::Zerg_Larva) ? distributeLarva(unit) : false;
}

void LarvaCode::onDrawPlan()
{
	//for (auto it : reservedlarvas)
	//	it->onDrawPlan();
}

void LarvaCode::onCheckMemoryLeaks()
{
	LarvaAgent::checkObjectsAlive();
	LarvaJob::checkObjectsAlive();
	LarvaPrecondition::checkObjectsAlive();
	HatcheryPlaner::checkObjectsAlive();
}
