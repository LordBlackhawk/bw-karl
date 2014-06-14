#ifndef MORPHBUILDINGTASK_h
#define MORPHBUILDINGTASK_h

#include "task.h"

class MorphBuildingTask : public Task
{
public:
	BWAPI::UnitType type;
	
	MorphBuildingTask(Task* o, int p, BWAPI::UnitType t)
		: Task(o, p), type(t)
	{
		//std::cerr << "whatBuilds: " << type.whatBuilds().first.getName() << std::endl;
		checkStatus();
	}
	
public:
	virtual int getNumberOfNeededAgents() const
	{ 
		return 1; 
	}
		
	virtual int getAgentValue(const Agent* agent) const
	{ 	
		if (type.whatBuilds().first != agent->unit->getType())
			return -1;

		return priority;
	}
	
	virtual void agentAssigned(Agent* agent)
	{ 
		Task::agentAssigned(agent);
		checkStatus();
	}
	
	virtual void agentDisassigned(Agent* agent) 
	{ 
		Task::agentDisassigned(agent);
		checkStatus();
	}
	
	virtual void setNeededResources()
	{
		if (res != NULL) {
			res->minerals = type.mineralPrice();
			res->gas      = type.gasPrice();
			res->supply   = 0;
		}
	}
	
	virtual void resourcesAssigned()
	{
		Task::resourcesAssigned();
		checkStatus();
	}

	virtual void agentFinishedJob(Agent* agent, const Job& job)
	{
		if (job == Job::Morph) {
			newStatus(TaskStatus::Completed);
		}
	}

private:
	void checkStatus()
	{		
		if (agents.size() <= 0) {
			newStatus(TaskStatus::WaitingForAgents);
			return;
		}
		
		if (res == NULL) {
			newStatus(TaskStatus::WaitingForResources);
			return;
		}
		
		if (status != TaskStatus::Preforming) {
			Agent* agent = *agents.begin();
			agent->clearJobs();
			agent->doJob(Job::morph(type, res));
			newStatus(TaskStatus::Preforming);
		}
	}
};

#endif