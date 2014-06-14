#ifndef OVERLORDSCOUTTASK_h
#define OVERLORDSCOUTTASK_h

#include "task.h"

class OverlordScoutTask : public Task
{
public:
	PositionList points;

public:
	OverlordScoutTask(Task* o, int p) : Task(o, p)
	{
		checkStatus();
	}

	virtual const char* getName() const
	{
		return "OverlordScout";
	}

	virtual int getNumberOfNeededAgents() const
	{
		return (int)points.size() + (int)agents.size();
	}

	virtual int getAgentValue(const Agent* agent) const
	{
		return (agent->isOverlord()) ? priority : -1;
	}

	virtual void agentAssigned(Agent* agent)
	{
		Task::agentAssigned(agent);
		agent->clearJobs();
		agent->doJob(Job::goTo(points.front()));
		points.pop_front();
		checkStatus();
		Broodwar->printf("sending overlord to scoutpoint!!");
	}

	void checkStatus()
	{
		if (points.size() > 0)
			newStatus(TaskStatus::WaitingForMoreAgentsWhilePreforming);
		else
			newStatus(TaskStatus::Preforming);
	}
};

#endif