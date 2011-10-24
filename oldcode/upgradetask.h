#ifndef UPGRADETASK_h
#define UPGRADETASK_h

#include "task.h"

class UpgradeTask : public Task
{
public:
	BWAPI::UpgradeType  type;
	
	UpgradeTask(Task* o, int p, BWAPI::UpgradeType t)
		: Task(o, p), type(t)
	{
		//std::cerr << "Waiting for agent '" << type.whatUpgrades().getName() << "'.\n";
		checkStatus();
	}
	
public:
	virtual const char* getName() const
	{
		return "Upgrade";
	}

	virtual int getNumberOfNeededAgents() const
	{ 
		return 1; 
	}
		
	virtual int  getAgentValue(const Agent* agent) const
	{
		//std::cerr << "Offered: " << agent->unit->getType().getName() << "\n";

		if (type.whatUpgrades() != agent->unit->getType())
			return -1;

		//log("Found something!");

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
		if (me->getUpgradeLevel(type) > 0) {
			newStatus(TaskStatus::Completed);
			return;
		}

		if (me->isUpgrading(type)) {
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
			agent->doJob(Job::upgrade(type, res));
			newStatus(TaskStatus::Preforming);
		}
	}
};

#endif