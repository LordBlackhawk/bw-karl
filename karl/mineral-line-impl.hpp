#pragma once

#include "container-helper.hpp"
#include <vector>

struct Agent;

std::vector<Agent*>		listAgents;
std::vector<Agent*>		newAgents;

struct Agent
{
	// Pure informations:
	//  * used by hungarian algorithm => maybe parallel => change only in update process.
	int 				time;
	Position			pos;
	Race				race;
	
	// Additional informations:
	//  * not used by hungarian algorithm => maybe used anytime by main thread.
	//  * pre != NULL 					=> waiting for unit.
	//  * pre == NULL && unit != NULL 	=> unit available.
	//  * pre == NULL && unit == NULL   => unit not available anymore.
	UnitPrecondition*	pre;
	Unit*				unit;
	
	Agent(UnitPrecondition* p)
		: time(p->time), pos(p->pos), race(p->ut.getRace()), pre(p), unit(NULL)
	{
		newAgents.push_back(this);
	}
	
	Agent(Unit* u)
		: time(0), pos(u->getPosition()), race(u->getType().getRace()), pre(NULL), unit(u)
	{
		newAgents.push_back(this);
	}
	
	bool waiting() const
	{
		return (pre != NULL);
	}
	
	bool exists() const
	{
		return (pre != NULL) || (unit != NULL);
	}
	
	bool update()
	{
		if (!exists())
			return true;
		
		if (pre != NULL) {
			if (pre->isFulfilled()) {
				time = 0;
				unit = pre->unit;
				race = unit->getType().getRace();
				release(pre);
			} else {
				time = pre->time;
				pos  = pre->pos;
				race = pre->ut.getRace();
			}
		}
		
		if (unit != NULL)
			pos  = unit->getPosition();
			
		return false;
	}
	
	void remove()
	{
		release(pre);
		unit = NULL;
	}
};

void updateAgents()
{
	listAgents.insert(listAgents.end(), newAgents.begin(), newAgents.end());
	newAgents.clear();
	Containers::remove_if(listAgents, std::fun_mem(Agent::update));
}

struct Job;

std::vector<Job*>		listJobs;
std::vector<Job*>		newJobs;

struct WorkerPrecondition;
struct MineralLine;
struct GasLine;

struct Job
{
	enum JobType { JobMineralLine, JobGasLine, JobExternal };
	
	// Pure informations:
	//  * used by hungarian algorithm => maybe parallel => change only in update process.
	int						type;
	int						wishtime;
	Position				wishpos;
	Race					wishrace;
	
	// Additional informations:
	Agent*					assigned;
	union
	{
		void*				ptr;
		MineralLine*		mineralline;
		GasLine*			gasline;
		WorkerPrecondition* external;
	};
	
	Job(MineralLine* m)
		: type(JobMineralLine), assigned(NULL), mineralline(m)
	{
		newJobs.push_back(this);
	}
	
	Job(GasLine* g)
		: type(JobGasLine), assigned(NULL), gasline(g)
	{
		newJobs.push_back(this);
	}
	
	Job(WorkerPreconditino* p)
		: type(JobExternal), assigned(NULL), external(p)
	{
		newJobs.push_back(this);
	}
	
	bool waiting() const
	{
		return (assigned == NULL) || assigned->waiting();
	}
	
	bool exists() const
	{
		return (ptr != NULL);
	}
	
	bool update();
	
	void remove(bool withworker = false)
	{
		ptr = NULL;
		if (withworker && !waiting())
			assigned->remove();
	}
};

void updateJobs()
{
	listJobs.insert(listJobs.end(), newJobs.begin(), newJobs.end());
	newJobs.clear();
	Containers::remove_if(listJobs, std::fun_mem(Job::update));
}

struct WorkerPrecondition : public UnitPrecondition
{
	Job*	job;
	
	WorkerPrecondition(const Race& r)
		: UnitPrecondition(Precondition::Impossible, r.getWorker(), Positions::Unknown)
	{
		job = new Job(this);
		update();
	}
	
	~WorkerPrecondition()
	{
		job->remove(true);
	}
	
	void updateJob()
	{
		job->wishtime = wishtime;
		job->wishpos  = Positions::Unknown;
		job->wishrace = ut.getRace();
		if (job->assigned != NULL) {
			time = job->assigned->time;
			unit = job->assigned->unit;
		} else {
			time = Precondition::Impossible;
			unit = NULL;
		}
	}
};

template <class Derived>
struct WorkerLine
{
	UnitPrecondition*	pre;
	Unit*				base;
	std::vector<Job*> 	jobs;
	
	WorkerLine(UnitPrecondition* p)
		: pre(p), base(NULL)
	{ }
	
	WorkerLine(Unit* u)
		: pre(NULL), base(u)
	{ }
	
	~WorkerLine()
	{
		for (auto it : jobs)
			it->remove(false);
	}
	
	Derived* This()
	{
		return static_cast<Derived*>(this);
	}
	
	void sendWorker(Unit* w)
	{
		if (!w->rightClick(This()->getTarget(w)))
			LOG << "Sending worker, but recieved error: " << Broodwar->getLastError().toString();
	}
	
	int estimateProduction() const
	{
		return 45 * worker.size();
	}
	
	void onActivated()
	{ }
	
	bool update()
	{
		if (pre != NULL) {
			if (!pre->isFulfilled())
				return false;
			
			base = pre->unit;
			release(pre);
			This()->onActivated();
			return true;
		}
		return false;
	}
	
	void addJob()
	{
		Job* job      = new Job(This());
		job->wishpos  = pre->pos;
		job->wishrace = Position::Unknown;
		jobs.push_back(job);
	}
	
	void updateJob(Job* job)
	{
		if (pre != NULL) {
			job->wishtime = pre->time;
		} else {
			job->wishtime = 0;
		}
	}
};

struct GasLine;

struct MineralLine : public WorkerLine<MineralLine>, public ObjectCounter<MineralLine>
{
	BWTA::BaseLocation*		location;
	std::set<GasLine*>		gaslines;
	
	MineralLine(UnitPrecondition* p, BWTA::BaseLocation* l)
		: WorkerLine<MineralLine>(p), location(l)
	{ }
	
	MineralLine(Unit* b, BWTA::BaseLocation* l)
		: WorkerLine<MineralLine>(b), location(l)
	{ }
	
	Unit* getTarget(Unit* w) const
	{
		return getNearest(location->getMinerals(), w->getPosition());
	}
	
	bool updateActive()
	{
		return !base->exists();
	}
	
	Unit* getUnusedGeyser() const;
	bool isYourGeyser(const Position& pos) const;
};

struct GasLine : public WorkerLine<GasLine>, public ObjectCounter<GasLine>
{
	MineralLine*		mineralline;
	int 				init_count;

	GasLine(UnitPrecondition* p, MineralLine* b, int c)
		: WorkerLine<GasLine>(p), mineralline(b), init_count(c)
	{
		mineralline->gaslines.insert(this);
		
		for (auto it : Broodwar->getStaticGeysers())
			if (it->getTilePosition() == pre->pos) {
				base = it;
				break;
			}
		
		if (base == NULL)
			LOG << "No Geyser found!";
	}
	
	~GasLine()
	{
		mineralline->gaslines.erase(this);
	}
	
	Unit* getTarget(Unit* /*w*/) const
	{
		return base;
	}
	
	bool updateActive()
	{
		return !base->getType().isRefinery();
	}
	
	void onActivated()
	{
		LOG << "Refinery build!";
		for (int k=0; k<init_count; ++k)
			incWorker();
	}
	
	void incWorker()
	{
		Unit* w = mineralline->getNearestWorker(base->getPosition());
		if (w != NULL)
			addWorker(w);
		else
			LOG << "No nearest worker!";
	}
};

Unit* MineralLine::getUnusedGeyser() const
{
	std::set<Unit*> geysers = location->getGeysers();
	for (auto it : gaslines)
		geysers.erase(it->base);
	if (geysers.empty())
		return NULL;
	return *geysers.begin();
}

bool MineralLine::isYourGeyser(const Position& pos) const
{
	for (auto it : location->getGeysers()) {
		LOG << "pos: " << pos.x() << "," << pos.y() << " =?= " << it->getPosition().x() << "," << it->getPosition().y();
		if (pos.getDistance(it->getPosition()) < 72.0)
			return true;
	}
	return false;
}