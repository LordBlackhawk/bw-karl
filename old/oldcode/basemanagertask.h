#ifndef BASEMANAGERTASK_h
#define BASEMANAGERTASK_h

#include "task.h"
#include "gathermineralstask.h"

class BaseManagerTask : public Task
{
public:
	BWTA::BaseLocation* base;

public:
	BaseManagerTask(Task* o, int p, BWTA::BaseLocation* b)
		: Task(o, p), base(b)
	{
		//std::cerr << this << ": "; log("BaseManager created!");
		newStatus(TaskStatus::WaitingForAgents);
	}

	static const char* getNameStatic()
	{
		return "BaseManager";
	}

	virtual const char* getName() const
	{
		return getNameStatic();
	}

	virtual int getNumberOfNeededAgents() const
	{ 
		return 1; 
	}

    virtual int getAgentValue(const Agent* agent) const 
	{ 
		if (!agent->isBaseAgent())
			return -1;

		/*
		std::cerr << "base location: " << base->getTilePosition().x() << "; "
			<< "unit location: " << agent->unit->getTilePosition().x() << "; "
			<< "distance: " << agent->unit->getTilePosition().getDistance(base->getTilePosition())
			<< std::endl;
			*/

		if (agent->unit->getTilePosition().getDistance(base->getTilePosition()) >= 3.0)
			return -1;

		return priority; 
	}
	
	virtual void agentAssigned(Agent* agent)
	{
		Task::agentAssigned(agent);
		new GatherMineralsTask(this, Priority::GatherMinerals, base);
		//std::cerr << this << ": "; log("BaseManager: Base assigned!");
		newStatus(TaskStatus::Preforming);
	}

	virtual void agentDisassigned(Agent* agent)
	{
		Task::agentDisassigned(agent);
		//std::cerr << this << ": "; log("BaseManager: Base disassigned!");

		if (status.isFinal())
			return;

		if (agents.size() <= 0) {
			//newStatus(TaskStatus::WaitingForAgents);
			//if (status == TaskStatus::WaitingForAgents)
			newStatus(TaskStatus::Failed);
		}
	}
};


#endif