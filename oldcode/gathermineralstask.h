#ifndef GATHERMINERALSTASK_h
#define GATHERMINERALSTASK_h

#include "mixed.h"
#include "task.h"
#include "helper.h"
#include "missingunitbuildtask.h"

class GatherMineralsTask : public MissingUnitBuildTask
{
public:
	BWTA::BaseLocation* location;

public:
	GatherMineralsTask(Task* o, int p, BWTA::BaseLocation* loc)
		: MissingUnitBuildTask(o, p, Broodwar->self()->getRace().getWorker(), 3),
		  location(loc)
	{
		checkStatus();
	}

	virtual const char* getName() const
	{
		return "GatherMinerals";
	}

	virtual int getNumberOfNeededAgents() const
	{
		return 2 * (int) location->getMinerals().size();
	}

	virtual int getAgentValue(const Agent* agent) const
	{	
		if (!agent->isWorker())
			return -1;

		return priority + agentDistanceToPrio(agent, bestMinerals()->getPosition());
	}

	virtual void agentAssigned(Agent* agent)
	{
		MissingUnitBuildTask::agentAssigned(agent);
		agent->clearJobs();
		agent->doJob(Job::gatherMinerals(bestMinerals()));
		checkStatus();
	}

	virtual void agentDisassigned(Agent* agent)
	{
		MissingUnitBuildTask::agentDisassigned(agent);
		checkStatus();
	}

	virtual void computeActions(int fcount)
	{
		MissingUnitBuildTask::computeActions(fcount);
		if (fcount % 101 == 56)
			checkStatus();
	}

private:
	void checkStatus()
	{
		if ((int)agents.size() >= getNumberOfNeededAgents())
			newStatus(TaskStatus::Preforming);
		else
			newStatus(TaskStatus::WaitingForMoreAgentsWhilePreforming);
	}

	Unit* bestMinerals() const
	{
		return getRandomSomething(location->getMinerals());
	}
};

#endif