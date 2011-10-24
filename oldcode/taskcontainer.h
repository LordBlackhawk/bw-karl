#ifndef TASKCONTAINER_h
#define TASKCONTAINER_h

#include "task.h"
#include "mixed.h"

class TaskContainer : public TaskSet
{
public:
	void moveAllTo(Task* newowner)
	{
		for (TaskSet::iterator it=begin(), itend=end(); it!=itend; ++it)
		{
			(*it).setNewParent(newowner);
		}
		clear();
	}
};

#endif