#ifndef EXTRACTORTRICKTASK_h
#define EXTRACTORTRICKTASK_h

#include "task.h"

class ExtractorTrickTask : public Task
{
public:
	Unit*			geyser;
	TilePosition	tileposition;
	Task*			buildtask;

public:
	ExtractorTrickTask(Task* o, int p, Unit* g)
		: Task(o, p), geyser(g), tileposition(g->getTilePosition()), buildtask(NULL)
	{
		newStatus(TaskStatus::Preforming);
	}

	virtual const char* getName() const
	{
		return "ExtractorTrick";
	}

	virtual void computeActions(int fcount)
	{
		Task::computeActions(fcount);

		Player* me    = Broodwar->self();
		int supTotal  = me->supplyTotal();
		int supUsed   = me->supplyUsed();

		if (supTotal >= 2*10) {
			if (buildtask != NULL) {
				buildtask->terminate();
				buildtask = NULL;
			}
			newStatus(TaskStatus::Completed);
			log("Extractor Trick finished!");
			return;
		}

		switch (status.type)
		{
		case TaskStatus::Preforming:
			if (buildtask != NULL)
				break;

			if (supUsed == supTotal) {
				buildtask = new BuildAtTask(this, priority, UnitTypes::Zerg_Extractor, tileposition);
				newStatus(TaskStatus::WaitingForSubtask);
			}	

			break;

		case TaskStatus::WaitingForSubtask:
			if (supUsed < supTotal) {
				new MorphUnitTask(this, priority, UnitTypes::Zerg_Drone);
				newStatus(TaskStatus::WaitingForEvent);
			} else 

			break;

		case TaskStatus::WaitingForEvent:
			if (supUsed == supTotal) {
				if (buildtask != NULL) {
					buildtask->terminate();
					buildtask = NULL;
				}
			} else if (supUsed > supTotal) {
				newStatus(TaskStatus::Preforming);
			}
			break;
		}
	}

	virtual void subtaskChangedStatus(Task* task)
	{
		if ((task == buildtask) && (task->status == TaskStatus::Completed)) 
		{
			Task::subtaskChangedStatus(task);
			newStatus(TaskStatus::Failed);
			log("Extractor Trick Task failed!");
			return;
		}

		if ((task == buildtask) && (task->status.isFinal()))
			buildtask = NULL;

		Task::subtaskChangedStatus(task);
	}
};


#endif