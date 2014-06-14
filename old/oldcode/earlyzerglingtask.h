#ifndef EARLYZERGLINGTASK_h
#define EARLYZERGLINGTASK_h

#include "task.h"

class EarlyZerglingTask : public Task
{
public:
	EarlyZerglingTask(Task* o, int p)
		: Task(o, p)
	{
		newStatus(TaskStatus::WaitingForMoreAgentsWhilePreforming);
	}

	virtual const char* getName() const
	{
		return "EarlyZergling";
	}

	virtual int getNumberOfNeededAgents() const
	{
		return 10;
	}

    virtual int getAgentValue(const Agent* agent) const
	{
		if (!agent->isZergling())
			return -1;

		return priority;
	}

    virtual void agentAssigned(Agent* agent)
	{
		Task::agentAssigned(agent);
		agentFinishedJob(agent, Job::ScoutPosition);
	}

	virtual void agentFinishedJob(Agent* agent, const Job& job)
	{
		agent->clearJobs();
		UnitSet units = Broodwar->getUnitsInRadius(agent->unit->getPosition(), UnitTypes::Zerg_Zergling.sightRange());
		Unit* target = getBestTarget(units);
		if (target != NULL) {
			agent->doJob(Job::attackNoDamage(target));
		} else {
			agent->doJob(Job::scoutPosition(getScoutPosition()));
		}
	}

private:
	bool betterTarget(Unit* t1, Unit* t2) const
	{
		if (t1 == NULL)
			return false;

		Player* player = t1->getPlayer();
		if (player == NULL)
			return false;

		if (player->isNeutral())
			return false;

		Player* me = Broodwar->self();
		if (me->isAlly(player))
			return false;

		UnitType type = t1->getType();
		if (type.isWorker())
			return true;

		if (!type.isBuilding())
			return false;

		if (t2 == NULL)
			return true;

		if (t2->getType().isWorker())
			return false;

		return true;
	}

	Unit* getBestTarget(const UnitSet& units) const
	{
		Unit *res = NULL;
		for (UnitSet::const_iterator it=units.begin(), itend=units.end(); it!=itend; ++it)
		{
			if (betterTarget(*it, res))
				res = *it;
		}
		return res;
	}

	Position getScoutPosition() const
	{
		BWTA::BaseLocation* base = getRandomSomething(BWTA::getBaseLocations());
		if (base != NULL)
			return base->getPosition();
		else
			return Positions::None;
	}
};

#endif