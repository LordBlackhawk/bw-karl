#ifndef GATHERGASTASK_h
#define GATHERGASTASK_h

#include "mixed.h"
#include "task.h"

#include "buildattask.h"

class GatherGasTask : public Task
{
public:
	BWTA::BaseLocation*	location;
	Unit* geyser;

public:
	GatherGasTask(Task* o, int p, BWTA::BaseLocation* loc)
		: Task(o, p), location(loc), geyser(NULL)
	{
		checkStatus();
	}

	virtual const char* getName() const
	{
		return "GatherGas";
	}

	virtual int getNumberOfNeededAgents() const
	{
		return 3;
	}

	virtual int getAgentValue(const Agent* agent) const
	{
		if (!agent->isWorker())
			return -1;

		Unit* mygeyser = geyser;
		if ((mygeyser == NULL) || !mygeyser->exists())
			mygeyser = findGeyser();
		int bonus = 0;
		if (mygeyser != NULL)
			bonus = agentDistanceToPrio(agent, mygeyser->getPosition());

		return priority + bonus;
	}

	virtual void agentAssigned(Agent* agent)
	{
		Task::agentAssigned(agent);

		if ((geyser == NULL) || !geyser->exists())
			geyser = findGeyser();
		if (geyser != NULL) {
			agent->clearJobs();
			agent->doJob(Job::gatherGas(geyser));
		}
		checkStatus();
	}

	virtual void agentDisassigned(Agent* agent)
	{
		Task::agentDisassigned(agent);
		checkStatus();
	}

	virtual void subtaskChangedStatus(Task* task)
	{
		TaskStatus substatus = task->status;
		Task::subtaskChangedStatus(task);

		if (substatus == TaskStatus::Failed) {
			newStatus(TaskStatus::Preforming);
			checkStatus();
			return;
		}

		if (substatus == TaskStatus::Completed)
			checkStatus();
	}

	virtual void computeActions(int fcount)
	{
		Task::computeActions(fcount);
		if (fcount % 101 == 56)
			checkStatus();
	}

private:
	void checkStatus()
	{
		if ((geyser == NULL) || !geyser->exists())
			geyser = findGeyser();

		if (geyser == NULL) {
			newStatus(TaskStatus::Failed);
			return;
		}

		if (   (geyser->getType() == UnitTypes::Resource_Vespene_Geyser)
		    || !geyser->getType().isRefinery() )
		{
			if (subtasks.size() <= 0) {
				UnitType refinery = Broodwar->self()->getRace().getRefinery();
				new BuildAtTask(this, priority, refinery, geyser->getTilePosition());
			}
			newStatus(TaskStatus::WaitingForSubtask);
		} else {
			if (geyser->isBeingConstructed()) {
				newStatus(TaskStatus::WaitingForSubtask);
				return;
			}

			if ((int)agents.size() >= getNumberOfNeededAgents())
				newStatus(TaskStatus::Preforming);
			else
				newStatus(TaskStatus::WaitingForMoreAgentsWhilePreforming);
		}
	}

	Unit* findGeyser() const
	{
		if (location->getGeysers().size() <= 0)
		{
			return NULL;
		}

		Unit* geyser = NULL;
		UnitSet::const_iterator it=location->getGeysers().begin(), itend=location->getGeysers().end();
		do {
			geyser = (*it);
			++it;
		} while ((it != itend) && !geyser->exists()) ;

		if (geyser->exists())
			return geyser;

		return searchForGeyser();
	}

	Unit* searchForGeyser() const // Bugfix!
	{
		UnitSet all = Broodwar->getUnitsInRadius(location->getPosition(), 400);
		for (UnitSet::iterator it=all.begin(), itend=all.end(); it!=itend; ++it)
		{
			Unit* unit = *it;
			if (unit->getType().isRefinery() || (unit->getType() == UnitTypes::Resource_Vespene_Geyser))
				return unit;
		}
		return NULL;
	}
};

#endif