#ifndef TASK_h
#define TASK_h

#include "mixed.h"
//#include "resources.h"
#include "taskstatus.h"

class Resources;

class Task
{
  public:
    TaskStatus		status;
    Task*			parent;
    TaskSet			subtasks;
    AgentSet 		agents;
    int 			priority;
    Resources*		res;
    
  public:
	Task(Task* o, int p)
		: parent(o), priority(p), res(NULL)
    {
		if (parent != NULL)
			parent->subtasks.insert(this);
	}
    
    ~Task();
    void newStatus(TaskStatus ts);
	void reassignAgents();
    
  public:
    virtual void computeActions(int fcount)
	{
		for (TaskSet::iterator it=subtasks.begin(), itend=subtasks.end(); it!=itend; ++it)
		{
			try {
				(*it)->computeActions(fcount);
			} catch (...) {
				Broodwar->printf("Exception in %s::computeActions()!",
									(*it)->getName());
			}
		}
	}

	virtual const char* getName() const
	{
		return "Task";
	}

    virtual int  getNumberOfNeededAgents() const { return 0; }
    virtual int  getAgentValue(const Agent* agent) const { return -99; }

    virtual void agentAssigned(Agent* agent) { agents.insert(agent); }
    virtual void agentDisassigned(Agent* agent) { agents.erase(agent); }

    virtual void setNeededResources() { };
    virtual void resourcesAssigned();
    
    virtual void agentFinishedJob(Agent* agent, const Job& job) { }
	virtual void agentFailedJob(Agent* agent, const Job& job) { }
    virtual void subtaskChangedStatus(Task* task)
	{
		if (task->status.isFinal())
			delete task;
	}

	virtual void terminate()
	{
		newStatus(TaskStatus::Failed);
	}

public:
	int agentDistanceToPrio(const Agent* agent, Position pos) const;
	void drawTaskInfo(int sx, int& sy) const;
	void drawAgentsInfo(int sx, int& sy, AgentSet as) const;
};

#endif