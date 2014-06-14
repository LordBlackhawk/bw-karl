#ifndef MISSINGUNITBUILDTASK_h
#define MISSINGUNITBUILDTASK_h

#include "task.h"
#include "morphunittask.h"

class MissingUnitBuildTask : public Task
{
public:
	UnitType	unittype;
	int			maxparallel;
	Position	pos;

public:
	MissingUnitBuildTask(Task* o, int p, UnitType ut, int mp, Position pos = Positions::None)
		: Task(o, p), unittype(ut), maxparallel(mp), pos(pos)
	{ }

	virtual void computeActions(int fcount)
	{
		Task::computeActions(fcount);
		if (fcount % 31 == 1)
			decideBuild();
	}

	virtual void agentAssigned(Agent* agent)
	{
		Task::agentAssigned(agent);
		decideBuild();
	}

	virtual void agentDisassigned(Agent* agent)
	{
		Task::agentDisassigned(agent);
		decideBuild();
	}

	virtual void subtaskChangedStatus(Task* task)
	{
		Task::subtaskChangedStatus(task);
		if (task->status.isFinal())
			decideBuild();
	}

private:
	void decideBuild()
	{
		if ((int)subtasks.size() >= maxparallel)
			return;

		int missing = getNumberOfNeededAgents() - agents.size() - subtasks.size();
		if (missing <= 0)
			return;

		Player* me = Broodwar->self();
		if (unittype.isBuilding()) {
			// new BuildSomewhereTask(this, unittype);
		} else {
			if (me->getRace() == Races::Zerg) {
				new MorphUnitTask(this, priority, unittype, pos);
			} else {
				// new TrainUnitTask(this, unittype);
			}
		}
	}

};

#endif