#ifndef MORPHUNITTASK_h
#define MORPHUNITTASK_h

#include "mixed.h"
#include "task.h"

class MorphUnitTask : public Task
{
public:
	UnitType 		unittype;
	Position		near_pos;

public:
	MorphUnitTask(Task* o, int p, UnitType ut, Position near = Positions::None)
		: Task(o, p), unittype(ut), near_pos(near)
	{
		newStatus(TaskStatus::WaitingForAgents);
	}

	virtual const char* getName() const
	{
		return "MorphUnit";
	}

	virtual int getNumberOfNeededAgents() const
	{
		return 1;
	}

	virtual int getAgentValue(const Agent* agent) const
	{
		if (!agent->isLarva())
			return -1;

		return priority + agentDistanceToPrio(agent, near_pos);
	}

	virtual void agentAssigned(Agent* agent)
	{
		Task::agentAssigned(agent);
		//Broodwar->printf("Agent assigned, waiting for Resources!");
		newStatus(TaskStatus::WaitingForResources);
	}

	virtual void agentDisassigned(Agent* agent)
	{
		Task::agentDisassigned(agent);
		if (!status.isFinal()) {
			if (agents.size() < 1) {
				//Broodwar->printf("Going back to WaitingForAgent!");
				newStatus(TaskStatus::WaitingForAgents);
			}
		}
	}

	virtual void setNeededResources()
	{
		res->minerals = unittype.mineralPrice();
		res->gas      = unittype.gasPrice();
		res->supply   = unittype.supplyRequired();
		//Broodwar->printf("Morph Resources aquired! (%d, %d, %d)",
		//		res->minerals, res->gas, res->supply);
		//std::cerr << "Morph: Resources aquired: " << res->minerals << std::endl;
	}

	virtual void resourcesAssigned()
	{
		if (agents.size() < 1) {
			AgentTaskResourcesManager::getInstance().onResourcesUsed(res);
			return;
		}

		Agent* agent = *agents.begin();
		agent->clearJobs();
		agent->doJob(Job::morph(unittype, res));

		newStatus(TaskStatus::Preforming);
	}

	virtual void agentFinishedJob(Agent* agent, const Job& job)
	{
		if (job == Job::Morph)
			newStatus(TaskStatus::Completed);
	}
};

#endif