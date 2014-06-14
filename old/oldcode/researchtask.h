#ifndef RESEARCHTASK_h
#define RESEARCHTASK_h

#include "task.h"

class ResearchTask : public Task
{
public:
	BWAPI::TechType  type;
	
	ResearchTask(Task* o, int p, BWAPI::TechType t)
		: Task(o, p), type(t)
	{
		//std::cerr << "Waiting for agent '" << type.whatResearches().getName() << "'.\n";
		checkStatus();
	}
	
public:
	virtual const char* getName() const
	{
		return "Research";
	}

	virtual int getNumberOfNeededAgents() const
	{ 
		return 1; 
	}
		
	virtual int  getAgentValue(const Agent* agent) const
	{
		if (type.whatResearches() != agent->unit->getType())
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

private:
	void checkStatus()
	{
		Player* me = Broodwar->self();
		if (me->hasResearched(type)) {
			newStatus(TaskStatus::Completed);
			return;
		}

		if (me->isResearching(type)) {
			newStatus(TaskStatus::Preforming);
			return;
		}
		
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
			agent->doJob(Job::research(type, res));
			newStatus(TaskStatus::Preforming);
		}
	}
};

#endif