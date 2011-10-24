#ifndef AGENTTASKMANAGER_h
#define AGENTTASKMANAGER_h

#include "mixed.h"
#include "resources.h"

class AgentTaskResourcesManager
{
public:
	static AgentTaskResourcesManager& getInstance()
	{
		static AgentTaskResourcesManager m;
		return m;
	}

	static AgentSet& getAllAgents()
	{
		return getInstance().allagents;
	}

private:
	AgentSet 		allagents;
	TaskSet 		waitingForResources;
	TaskSet			waitingForAgents;
	ResourcesSet	aquiredResources;
	ResourcesSet	reservedResources;

	AgentTaskResourcesManager() { }

public:
	void onFrame(int fcount)
	{
		Player* me = Broodwar->self();
		Resources current;
		current.minerals = me->minerals();
		current.gas = me->gas();
		current.supply = me->supplyTotal() - me->supplyUsed();
		shareResources(current);

		for (AgentSet::iterator it=allagents.begin(), itend=allagents.end(); it!=itend; ++it)
		{
			(*it)->computeActions(fcount);
		}
	}

	void onAddAgent(Agent* agent)
	{
		allagents.insert(agent);
		findTaskForAgent(agent);
		if (agent->subagent != NULL) {
			onAddAgent(agent->subagent);
		}
	}

	void onGiveBackAgent(Agent* agent)
	{
		assignAgentNew(agent, NULL);
		findTaskForAgent(agent);
	}

	void onRemoveAgent(Agent* agent)
	{
		allagents.erase(agent);
		if (agent->owner != NULL) {
			agent->owner->agentDisassigned(agent);
			agent->owner = NULL;
		}
		if (agent->subagent != NULL) {
			onRemoveAgent(agent->subagent);
		}
	}

	void onTaskNewStatus(Task* task, TaskStatus oldstatus, TaskStatus newstatus)
	{
		switch (oldstatus.type)
		{
		case TaskStatus::WaitingForResources:
			if (task->res != NULL)
				aquiredResources.erase(task->res);
			break;

		case TaskStatus::WaitingForAgents:
		case TaskStatus::WaitingForMoreAgentsWhilePreforming:
			waitingForAgents.erase(task);
			break;
		}

		switch (newstatus.type)
		{
		case TaskStatus::WaitingForResources:
			{
				Resources* res = new Resources(task);
				task->res = res;
				task->setNeededResources();
				aquiredResources.insert(res);
				break;
			}

		case TaskStatus::WaitingForAgents:
		case TaskStatus::WaitingForMoreAgentsWhilePreforming:
			waitingForAgents.insert(task);
			findAgentsForTask(task);
			break;
		}
	}

	void onResourcesUsed(Resources* res)
	{
		aquiredResources.erase(res);
		reservedResources.erase(res);
		if (res->owner != NULL)
			res->owner->res = NULL;
		delete res;
	}

private:
	int decide(Agent* agent, Task* task) const
	{
		int value = task->getAgentValue(agent);
		if (value < 0)
			return value;

		int ownervalue = (agent->owner != NULL) ? agent->owner->getAgentValue(agent) : -1;

		return value - ownervalue;
	}

	void assignAgentNew(Agent* agent, Task* task) const
	{
		if (agent->owner == task)
			return;

		Task* oldowner = agent->owner;
		agent->owner   = task;

		if (oldowner != NULL)
			oldowner->agentDisassigned(agent);

		if ((agent->owner == task) && (task != NULL))
			task->agentAssigned(agent);
	}

	void findAgentsForTask(Task* task)
	{
		//Broodwar->printf("findAgentsForTask called.");

		int needed = task->getNumberOfNeededAgents() - task->agents.size();
		if (needed <= 0)
			return;

		AgentCandidateList candidates;
		for (AgentSet::iterator it=allagents.begin(), itend=allagents.end(); it!=itend; ++it)
		{
			int value = decide(*it, task);
			if (value > 0)
				candidates.push(IntAgentPair(value, *it));
		}

		//Broodwar->printf("Candidates: %d", candidates.size());

		while ((needed > 0) && !candidates.empty())
		{
			Agent* agent = candidates.top().second;
			candidates.pop();
			assignAgentNew(agent, task);
			--needed;
		}
	}

	void findTaskForAgent(Agent* agent)
	{
		Task* bestBidder  = NULL;
		int   bestBidding = -1;

		for (TaskSet::iterator it=waitingForAgents.begin(), itend=waitingForAgents.end(); it!=itend; ++it)
		{
			if ((*it)->getNumberOfNeededAgents() <= (int)(*it)->agents.size())
				continue;

			int value = (*it)->getAgentValue(agent);
			if (value > bestBidding) {
				bestBidding = value;
				bestBidder  = *it;
			}
		}

		if (bestBidder != NULL)
			assignAgentNew(agent, bestBidder);
	}

	void shareResources(Resources current)
	{
		Resources reserved;
		for (ResourcesSet::iterator it=reservedResources.begin(), itend=reservedResources.end(); it!=itend; ++it)
		{
			reserved += *(*it);
		}
		current -= reserved;
		Broodwar->drawTextScreen(452, 16, "\x07%d", reserved.minerals);
		Broodwar->drawTextScreen(520, 16, "\x07%d", reserved.gas);
		Broodwar->drawTextScreen(588, 16, "\x07%d", (reserved.supply+1)/2);

		Resources aquired;
		for (ResourcesSet::iterator it=aquiredResources.begin(), itend=aquiredResources.end(); it!=itend; ++it)
		{
			aquired += *(*it);
		}
		Broodwar->drawTextScreen(452, 32, "\x07%d", aquired.minerals);
		Broodwar->drawTextScreen(520, 32, "\x07%d", aquired.gas);
		Broodwar->drawTextScreen(588, 32, "\x07%d", (aquired.supply+1)/2);

		ResourcesCandidateList candidates;
		for (ResourcesSet::iterator it=aquiredResources.begin(), itend=aquiredResources.end(); it!=itend; ++it)
		{
			candidates.push(IntResourcesPair((*it)->owner->priority, *it));
		}

		while (!candidates.empty()) {
			Resources* res = candidates.top().second;
			candidates.pop();
		//	std::cerr << "Have inenough " << current.minerals << " " << res->minerals << std::endl;
			if (*res <= current) {
				current -= *res;
				aquiredResources.erase(res);
				if (res->owner != NULL) {
					reservedResources.insert(res);
					res->owner->resourcesAssigned();
				}
			} else {
				current -= *res;
			}
		}
	}
};

#endif