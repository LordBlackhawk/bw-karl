#ifndef AGENT_h
#define AGENT_h

#include "mixed.h"
#include "job.h"
#include "task.h"
#include "agentdeleter.h"

class Agent
{ 
public:
	Agent*		subagent;
	Unit* 		unit;
	JobList   	jobs;
	Job			currentjob;
	Task*		owner;

public:
	Agent(Unit* u) : subagent(NULL), unit(u), owner(NULL) { }

	virtual ~Agent()
	{
		if (subagent != NULL) {
			delete subagent;
			subagent = NULL;
		}
	}

	void markToDelete()
	{
		AgentDeleter::deleteAgent(this);
	}

public:
	bool isAlive() const { return unit->exists(); }
	bool isWorker() const { return unit->getType().isWorker(); }
	bool isLarva() const { return unit->getType() == UnitTypes::Zerg_Larva; }
	bool isOverlord() const { return unit->getType() == UnitTypes::Zerg_Overlord; }
	bool isZergling() const { return unit->getType() == UnitTypes::Zerg_Zergling; }

	bool isHatch() const
	{
		return (unit->getType() == UnitTypes::Zerg_Hatchery)
			|| (unit->getType() == UnitTypes::Zerg_Lair)
			|| (unit->getType() == UnitTypes::Zerg_Hive);
	}

public:
	void clearJobs() { jobs.clear(); currentjob = Job::Reset; }
	void doJob(const Job& job) { jobs.push_back(job); }

	void nextJob()
	{
		if (   (currentjob != Job::Reset)
			&& (currentjob != Job::Waiting)  )
		{
			if (owner != NULL)
				owner->agentFinishedJob(this, currentjob);
		}

		if (jobs.size() > 0) {
			currentjob = jobs.front();
			jobs.pop_front();
		} else {
			currentjob = Job::Waiting;
		}
	}
	
	void jobFailed()
	{
		if (owner != NULL)
			owner->agentFailedJob(this, currentjob);

		if (jobs.size() > 0) {
			currentjob = jobs.front();
			jobs.pop_front();
		} else {
			currentjob = Job::Waiting;
		}
	}

public:
	virtual bool isBaseAgent() const { return false; }

	virtual void computeActions(int framecount) 
	{ 
		if (currentjob == Job::Reset)
			nextJob();
	}

	virtual bool onMorph() { return true; }
	virtual bool onDestroy() { return true; }
};

#endif