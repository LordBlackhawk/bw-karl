#ifndef BUILDATTASK_h
#define BUILDATTASK_h

#include "mixed.h"
#include "task.h"

class BuildAtTask : public Task
{
public:
	UnitType 		unittype;
	TilePosition 	tileposition;
	bool			early_worker;

public:
	BuildAtTask(Task* o, int p, UnitType ut, TilePosition tp, bool ew = false)
		: Task(o, p), unittype(ut), tileposition(tp), early_worker(ew)
	{
		checkStatus();
	}

	static const char* getNameStatic()
	{
		return "BuildAt";
	}

	virtual const char* getName() const
	{
		return getNameStatic();	
	}

	virtual int getNumberOfNeededAgents() const
	{
		return 1;
	}

	virtual int getAgentValue(const Agent* agent) const
	{
		if (!agent->isWorker())
			return -1;

		return priority + agentDistanceToPrio(agent, Position(tileposition));
	}

	virtual void agentAssigned(Agent* agent)
	{
		Task::agentAssigned(agent);
		checkStatus();
	}

	virtual void agentDisassigned(Agent* agent)
	{
		Task::agentDisassigned(agent);

		if (status.isFinal())
			return;

		checkStatus();
	}

	virtual void setNeededResources()
	{
		if (res != NULL) {
			res->minerals = unittype.mineralPrice();
			res->gas      = unittype.gasPrice();
			res->supply   = 0;
		}
	}

	virtual void resourcesAssigned()
	{
		checkStatus();
	}

	virtual void agentFinishedJob(Agent* agent, const Job& job)
	{
		if (job == Job::Build)
			newStatus(TaskStatus::Completed);
	}
	
	virtual void agentFailedJob(Agent* agent, const Job& job)
	{
		reassignAgents();
		// WICHTIG: Abgeleitete Klassen bestimmen selbst, was jetzt passiert!
		tileposition = (getName() == getNameStatic()) ? TilePositions::Invalid : TilePositions::Unknown;
		checkStatus();
	}

	virtual void terminate()
	{
		if (agents.size() > 0) {
			Agent* agent = *agents.begin();
			agent->clearJobs();
		}
		Task::terminate();
	}

protected:
	void checkStatus()
	{
		if (   (tileposition == TilePositions::Invalid)
			|| (tileposition == TilePositions::None) )
		{
			newStatus(TaskStatus::Failed);
		} else if (tileposition == TilePositions::Unknown) {
			newStatus(TaskStatus::WaitingForLocation);
		} else if (early_worker && (agents.size() <= 0)) {
			newStatus(TaskStatus::WaitingForAgents);
		} else if (res == NULL) {
			newStatus(TaskStatus::WaitingForResources);
		} else if (agents.size() <= 0) {
			newStatus(TaskStatus::WaitingForAgents);
		} else if (status != TaskStatus::Preforming) {
			std::cerr << "Sending Worker to build " << unittype.getName() << std::endl;
			Agent* agent = *agents.begin();
			agent->clearJobs();
			agent->doJob(Job::build(tileposition, unittype, res));
			newStatus(TaskStatus::Preforming);
		}
	}
};

#endif