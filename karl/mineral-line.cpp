// ToDo:
//  * estimatedProduction improving by looking into future.

#include "mineral-line.hpp"
#include "idle-unit-container.hpp"
#include "container-helper.hpp"
#include "hungarian-algorithm.hpp"
#include "parallel-vector.hpp"
#include "object-counter.hpp"
#include "unit-lifetime-observer.hpp"
#include "precondition-helper.hpp"
#include "building-placer.hpp"
#include "unit-builder.hpp"
#include "bwapi-helper.hpp"
#include "valuing.hpp"
#include "log.hpp"

#include <BWTA.h>

#include <vector>
#include <algorithm>
#include <functional>

using namespace BWAPI;
using namespace BWTA;

#define THIS_DEBUG LOG

std::vector<Production> estimatedProduction;

namespace
{
    struct WorkerAgent;
    struct WorkerJob;
    struct WorkerPrecondition;
    struct WorkerLine;
    
    ParallelVector<WorkerAgent*>        agents;
    ParallelVector<WorkerJob*>          jobs;
    std::vector<WorkerLine*>            lines;
    
    struct WorkerAgent : public ObjectCounter<WorkerAgent>
    {
        int                 time;
        Position            pos;
        Race                race;
        Unit*               worker;
        UnitPrecondition*   pre;
        WorkerJob*          assigned;
        bool                remove;
        
        WorkerAgent(Unit* u)
            : time(Precondition::Impossible), pos(u->getPosition()), race(u->getType().getRace()), worker(u), 
              pre(NULL), assigned(NULL), remove(false)
        {
            Containers::add(agents, this);
        }
        
        WorkerAgent(UnitPrecondition* p)
            : time(Precondition::Impossible), pos(p->pos), race(p->ut.getRace()), worker(NULL), 
              pre(p), assigned(NULL), remove(false)
        {
            Containers::add(agents, this);
        }
        
        ~WorkerAgent()
        {
            release(pre);
        }

        void markRemove()
        {
            remove = true;
        }
        
        bool update();
    };
    
    struct WorkerJob : public ObjectCounter<WorkerJob>
    {
        int                 wishtime;
        Position            wishpos;
        Race                wishrace;
        WorkerPrecondition* pre;
        WorkerLine*         line;
        WorkerAgent*        assigned;
        bool                remove;
        
        WorkerJob(WorkerPrecondition* p);
        WorkerJob(WorkerLine* l);
        
        void markRemove()
        {
            if (assigned != NULL) {
                assigned->markRemove();
                assigned = NULL;
            }
            pre    = NULL;
            remove = true;
        }
        
        void markRemoveJobOnly()
        {
            if (assigned != NULL) {
                assigned->assigned = NULL;
                assigned = NULL;
            }
            pre    = NULL;
            remove = true;
        }
        
        void commandWorker(WorkerAgent* agent);
        bool update();
    };
    
    struct WorkerPrecondition : public UnitPrecondition, public ObjectCounter<WorkerPrecondition>
    {
        WorkerJob*      job;
        
        WorkerPrecondition(const Race& r)
            : UnitPrecondition(Precondition::Impossible, r.getWorker(), Positions::Unknown)
        {
            job = new WorkerJob(this);
        }
        
        ~WorkerPrecondition()
        {
            job->markRemove();
        }
    };
    
    struct WorkerLine : public UnitLifetimeObserver<WorkerLine>, public ObjectCounter<WorkerLine>
    {
        std::vector<WorkerAgent*>       agents;
        std::vector<WorkerJob*>         jobs;
        BWTA::BaseLocation*             location;
        std::set<WorkerLine*>           gaslines;
        Unit*                           geyser;
        
        WorkerLine(BWTA::BaseLocation* l, Unit* u)
            : UnitLifetimeObserver<WorkerLine>(u), location(l), geyser(NULL)
        {
            lines.push_back(this);
        }
        
        WorkerLine(BWTA::BaseLocation* l, UnitPrecondition* p)
            : UnitLifetimeObserver<WorkerLine>(p), location(l), geyser(NULL)
        {
            lines.push_back(this);
        }
        
        WorkerLine(BWTA::BaseLocation* l, UnitPrecondition* p, Unit* g)
            : UnitLifetimeObserver<WorkerLine>(p), location(l), geyser(g)
        {
            lines.push_back(this);
        }
        
        void onRemoveFromList()
        {
            Containers::remove(lines, this);
        }
        
        void commandWorker(WorkerAgent* agent);
        void addJob();
        void removeJob();
        void updateMineralLineJobs();
        void onUnitReady();
        void onUnitDestroyed();
        void onUpdateOnline();
        void onUpdateOffline();
        Unit* getUnusedGeyser() const;
    };
    
    struct ProblemType
    {
        double evaluate(int idAgent, int idJob) const
        {
            if (idAgent >= agents.size()) {
                WorkerJob* job = jobs[idJob];
                bool isGasJob = false;
                bool isMineralJob = false;
                if (job->line != NULL) {
                    if (job->line->geyser != NULL)
                        isGasJob = true;
                    else
                        isMineralJob = true;
                }
                return valueWorkerAssignmentNoAgent(isGasJob, isMineralJob, job->wishtime);
            } else if (idJob >= jobs.size()) {
                WorkerAgent* agent = agents[idAgent];
                return valueWorkerAssignmentNoJob(agent->time);
            }

            WorkerAgent* agent = agents[idAgent];
            WorkerJob* job = jobs[idJob];
            
            bool assigned = false;
            if (agent->assigned != NULL) {
                if (agent->assigned->line != NULL) {
                    assigned = agent->assigned->line == job->line;
                } else {
                    assigned = job->assigned == agent;   
                }
            }
            bool isPlanedWorker = agent->pre != NULL;
            bool isGasJob = false;
            bool isMineralJob = false;
            if (job->line != NULL) {
                if (job->line->geyser != NULL)
                    isGasJob = true;
                else
                    isMineralJob = true;
            }
            
            return valueWorkerAssignment(isPlanedWorker, isGasJob, isMineralJob, 
                                         agent->time, job->wishtime,
                                         agent->pos, job->wishpos, assigned);
        }

        void assign(int idAgent, int idJob) const
        {
            if ((idJob < 0) || (idJob >= jobs.size())) {
                agents[idAgent]->assigned = NULL;
                return;
            }
            WorkerJob* job = jobs[idJob];

            if ((idAgent < 0) || (idAgent >= agents.size())) {
                job->assigned = NULL;
                return;
            }
            WorkerAgent* agent = agents[idAgent];
            
            bool changed = true;
            if (agent->assigned != NULL) {
                if (job->line != NULL) {
                    if (agent->assigned->line == job->line)
                        changed = false;
                } else {
                    changed = false;
                }
            }

            agent->assigned = job;
            job->assigned = agent;
            job->update();

            if (changed)
                job->commandWorker(agent);
        }

        int numberOfAgents() const
        {
            return agents.size();
        }

        int numberOfJobs() const
        {
            return jobs.size();
        }
    };
    
    BaseLocation* checkBaseLocation(const Position& pos)
    {
        BaseLocation* location = getNearestBaseLocation(pos);
        for (auto it : lines)
            if (it->location == location)
                return NULL;
        return location;
    }
    
    bool WorkerAgent::update()
    {
        if (remove) {
            delete this;
            return true;
        }
        
        if (pre != NULL)
            if (pre->isFulfilled())
        {
            worker = pre->unit;
            time   = 0;
            release(pre);
            if (assigned != NULL)
                assigned->commandWorker(this);
        }

        return false;
    }
    
    WorkerJob::WorkerJob(WorkerPrecondition* p)
        : wishtime(0), wishpos(Positions::Unknown), wishrace(p->ut.getRace()), pre(p), line(NULL), assigned(NULL), remove(false)
    {
        Containers::add(jobs, this);
    }
    
    WorkerJob::WorkerJob(WorkerLine* l)
        : wishtime(0), wishpos(Positions::Unknown), wishrace(Races::Unknown), pre(NULL), line(l), assigned(NULL), remove(false)
    {
        Containers::add(jobs, this);
    }
    
    void WorkerJob::commandWorker(WorkerAgent* agent)
    {
        if (line != NULL) {
            line->commandWorker(agent);
        } else {
            pre->time = 0;
            pre->unit = agent->worker;
        }
    }
    
    bool WorkerJob::update()
    {
        if (remove) {
            delete this;
            return true;
        }
        
        if (pre != NULL) {
            wishtime = pre->wishtime;
            if (assigned != NULL) {
                pre->time = assigned->time;
                pre->unit = assigned->worker;
            } else {
                pre->time = Precondition::Impossible;
                pre->unit = NULL;
            }
        }
        
        return false;
    }
    
    void WorkerLine::commandWorker(WorkerAgent* agent)
    {
        Unit* worker = agent->worker;
        if (worker == NULL)
            return;

        Unit* target = NULL;
        if (geyser != NULL) {
            target = geyser;
        } else {
            target = getNearest(location->getMinerals(), worker->getPosition());
        }
        worker->rightClick(target);
    }
    
    void WorkerLine::addJob()
    {
        jobs.push_back(new WorkerJob(this));
    }
    
    void WorkerLine::removeJob()
    {
        if (jobs.empty())
            return;
        jobs.back()->markRemoveJobOnly();
        jobs.erase(jobs.end()-1);
    }
    
    void WorkerLine::updateMineralLineJobs()
    {
        auto count = 2 * location->getStaticMinerals().size();
        while (count > jobs.size())
            addJob();
        while (count < jobs.size())
            removeJob();
    }
    
    void WorkerLine::onUnitReady()
    {
        if (geyser != NULL) {
            for (int k=0; k<3; ++k)
                addJob();
            LOG << "created refinery jobs...";
        } else {
            updateMineralLineJobs();
        }
    }
    
    void WorkerLine::onUnitDestroyed()
    {
        while (!jobs.empty())
            removeJob();
        //delete this;
    }
    
    void WorkerLine::onUpdateOnline()
    {
        if (geyser == NULL) {
            updateMineralLineJobs();
            Containers::remove_if(gaslines, std::mem_fun(&WorkerLine::update));
        }
    }
    
    void WorkerLine::onUpdateOffline()
    { }
    
    Unit* WorkerLine::getUnusedGeyser() const
    {
        std::set<Unit*> geysers = location->getGeysers();
		for (auto it : gaslines)
			geysers.erase(it->geyser);
		if (geysers.empty())
			return NULL;
		return *geysers.begin();
    }
    
    ProblemType                          problem;
    HungarianAlgorithm<ProblemType>      assignment(problem);
}

void useWorker(BWAPI::Unit* unit)
{
	new WorkerAgent(unit);
}

void useWorker(UnitPrecondition* unit)
{
	if (unit == NULL) {
		WARNING << "called useWorker with unit == NULL.";
		return;
	}

	if (unit->isFulfilled()) {
		THIS_DEBUG << "Sending worker immediately.";
		useWorker(unit->unit);
		release(unit);
		return;
	}
	
	new WorkerAgent(unit);
}

UnitPrecondition* getWorker(const BWAPI::Race& r)
{
    return new WorkerPrecondition(r);
}

void registerBase(Unit* u)
{
    THIS_DEBUG << "Base added.";
    BWTA::BaseLocation* location = checkBaseLocation(u->getPosition());
    if (location != NULL)
        new WorkerLine(location, u);
}

UnitPrecondition* registerBase(UnitPrecondition* b)
{
    THIS_DEBUG << "Planed Base added.";
    BWTA::BaseLocation* location = checkBaseLocation(b->pos);
    if (location != NULL) {
        return (new WorkerLine(location, b))->pre;
    } else {
        return b;
    }
}

bool buildRefinery(const BWAPI::UnitType& type)
{
    for (auto it : lines) {
		Unit* geyser = it->getUnusedGeyser();
		if (geyser == NULL)
			continue;
		
		BuildingPositionPrecondition* pos = getBuildingPosition(type, geyser->getTilePosition());
        auto result = buildUnit(pos, type);
        if (result.first == NULL)
            continue;
        
        if (result.second != NULL)
            useWorker(result.second);

        WorkerLine* gasline = new WorkerLine(it->location, result.first, geyser);
        it->gaslines.insert(gasline);
        return true;
	}
    
    return false;
}

void MineralLineCode::onMatchBegin()
{
    for (auto it : Broodwar->self()->getUnits()) {
        if (it->getType().isResourceDepot())
            registerBase(it);
        if (it->getType().isWorker())
            useWorker(it);
    }
    
    estimatedProduction.resize(1);
	Production& prod = estimatedProduction[0];
	prod.time     = 0;
	prod.minerals = 4*45;
	prod.gas      = 1;
}

void MineralLineCode::onMatchEnd()
{
    Containers::clear_and_delete(agents);
    Containers::clear_and_delete(jobs);
    Containers::clear_and_delete(lines);
}

void MineralLineCode::onTick()
{
    for (auto it : agents)
        if (it->worker != NULL)
            if (!it->worker->getType().isWorker())
    {
        rememberIdle(it->worker);
        it->markRemove();
    }

    Containers::remove_if(lines, std::mem_fun(&WorkerLine::update));

    agents.update();
    Containers::remove_if(agents, std::mem_fun(&WorkerAgent::update));

    jobs.update();
    Containers::remove_if(jobs, std::mem_fun(&WorkerJob::update));
    
    if (Broodwar->getFrameCount() % 100 == 0)
        LOG << "Solving assignment problem with " << agents.size() << " worker and " << jobs.size() << " jobs.";
    assignment.execute();
    
    // ToDo: Estimate Production!
}

void MineralLineCode::onCheckMemoryLeak()
{
    WorkerAgent::checkObjectsAlive();
    WorkerJob::checkObjectsAlive();
    WorkerPrecondition::checkObjectsAlive();
    WorkerLine::checkObjectsAlive();
}