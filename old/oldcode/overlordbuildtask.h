#ifndef OVERLORDBUILDTASK_h
#define OVERLORDBUILDTASK_h

#include "task.h"

class OverlordBuildTask : public Task
{
public:
	OverlordBuildTask(Task* o, int p) : Task(o, p)
	{
		newStatus(TaskStatus::Preforming);
	}

	virtual const char* getName() const
	{
		return "OverlordBuild";
	}

	virtual void computeActions(int fcount)
	{
		Task::computeActions(fcount);

		if (fcount % 29 == 27) {

			Player* me = Broodwar->self();
			int supTotal = me->supplyTotal();
			int supUsed  = me->supplyUsed();
			int supBuild = (int) subtasks.size();
			int supDiff  = supTotal - supUsed;

			bool build = false;
			if (supTotal <= 2*9) {
				if ((supUsed >= 2*8) && (supBuild < 1))
					build = true;
			} else {
				if ((supDiff < 2*3) && (supBuild < 1))
					build = true;
				else if ((supDiff < 1) && (supBuild < 2))
					build = true;
			}

			if (build) {
				new MorphUnitTask(this, priority, UnitTypes::Zerg_Overlord);
				Broodwar->printf("Build Overlord at %d %d %d %d",
					supTotal, supUsed, supBuild, supDiff);
			}
		}
	}
};

#endif