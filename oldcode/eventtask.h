#ifndef EVENTTASK_h
#define EVENTTASK_h

#include "task.h"
#include "containertask.h"

class EventTask : public Task
{
public:
	TaskContainer onFailedDo;
	TaskContainer onCompletedDo;
	TaskContainer onPreformingDo;
	
public:
	EventTask(Task* o) : Task(o, 0)
	{
		newStatus(TaskStatus::Preforming);
	}
	
	virtual ~EventTask()
	{ }

public:
	virtual const char* getName() const
	{
		return "Event";
	}
	
	// TODO Namen korrigieren!
	virtual void subtaskChangedStatus(Task* task)
	{		
		bool finish = false;
		if (task.status == TaskStatus::Failed) {
			onFailedDo.moveAllTo(parent);
			finish = true;
		} else if (task.status == TaskStatus::Completed) {
			onCompletedDo.moveAllTo(parent);
			finish = true;
		} else if (task.status == TaskStatus::Preforming) {
			onPreformingDo.moveAllTo(parent);
		}
		Task::subtaskChangedStatus(task);
		if (finish)
			newStatus(TaskStatus::Completed);
	}
	
};	

#endif