// ToDo:
//  * estimatedProduction improving by looking into future.
//  * Add setGasMode(None / Auto / AsRequired), setGasWorker(int), ...
//  * No idle workers.

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
#include "code-list.hpp"
#include <BWTA.h>
#include <sstream>
#include <set>
#include <vector>
#include <algorithm>
#include <functional>

using namespace BWAPI;
using namespace BWTA;

#define THIS_DEBUG DEBUG

std::vector<Production> estimatedProduction;

namespace
{
    int defaultGasWorkerNumber = 3;

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
            : time(0), pos(u->getPosition()), race(u->getType().getRace()), worker(u),
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

        void markRemove();
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
        void markRemoveWithAgent();
        void markRemoveJobOnly();
        void commandWorker(WorkerAgent* agent);
        void releaseWorker(WorkerAgent* agent);
        bool update();
    };

    struct WorkerPrecondition : public UnitPrecondition, public ObjectCounter<WorkerPrecondition>
    {
        WorkerJob*      job;

        WorkerPrecondition(const Race& r)
            : UnitPrecondition(Precondition::Impossible, r.getWorker(), Positions::Unknown, UnitPrecondition::WithoutAddon)
        {
            job = new WorkerJob(this);
        }

        ~WorkerPrecondition()
        {
            if (job != NULL) {
                job->markRemoveWithAgent();
                job = NULL;
            }
        }
    };

    struct WorkerLine : public UnitLifetimeObserver<WorkerLine>, public ObjectCounter<WorkerLine>
    {
        std::set<WorkerAgent*>          agents;
        std::vector<WorkerJob*>         jobs;
        BWTA::BaseLocation*             location;
        std::set<WorkerLine*>           gaslines;
        Unit*                           geyser;
        int                             mineralfields;

        WorkerLine(BWTA::BaseLocation* l, Unit* u)
            : UnitLifetimeObserver<WorkerLine>(u), location(l), geyser(NULL), mineralfields(0)
        {
            lines.push_back(this);
            updateMineralfields();
        }

        WorkerLine(BWTA::BaseLocation* l, UnitPrecondition* p)
            : UnitLifetimeObserver<WorkerLine>(p), location(l), geyser(NULL), mineralfields(0)
        {
            lines.push_back(this);
            updateMineralfields();
        }

        WorkerLine(BWTA::BaseLocation* l, UnitPrecondition* p, Unit* g)
            : UnitLifetimeObserver<WorkerLine>(p), location(l), geyser(g), mineralfields(0)
        {
            //lines.push_back(this);
        }

        bool isGasReady() const
        {
            return (geyser->getPlayer() == Broodwar->self() && geyser->isIdle());
        }

        ~WorkerLine()
        {
            while (!jobs.empty())
                removeJob();
        }
        
        void onRemoveFromList()
        {
            //assert(false && "Should never be called!");
        }
        
        void updateMineralfields()
        {
            int oldvalue = mineralfields;
            mineralfields = 0;
            for (auto it : location->getStaticMinerals())
                if (it->exists())
                    ++mineralfields;
            if (oldvalue != mineralfields) {
                THIS_DEBUG << "Changed mineralfields from " << oldvalue << " to " << mineralfields << ".";
                if (mineralfields == 0)
                    CodeList::onBaseMinedOut(location);
            }
        }
        
        int estimateMineralProd() const
        {
            return std::min(45 * (int) agents.size(), 60 * mineralfields);
        }
        
        int estimateSingleGasProd() const
        {
            return ((geyser->getResources() > 0) ? 72 : 18) * (int) agents.size();
        }
        
        int estimateGasProd() const
        {
            int sum = 0;
            for (auto it : gaslines)
                sum += it->estimateSingleGasProd();
            return sum;
        }

        void setJobCount(int c);
        void commandWorker(WorkerAgent* agent);
        void releaseWorker(WorkerAgent* agent);
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
        bool isOldJob(WorkerAgent* agent, WorkerJob* job) const
        {
            assert(agent != NULL);

            if (agent->assigned == job)
                return true;

            if ((job == NULL) || (agent->assigned == NULL))
                return false;

            return (agent->assigned->line == job->line);
        }

        ctype valueCombination(WorkerAgent* agent, WorkerJob* job) const
        {
            if (agent == NULL) {
                bool isGasJob = false;
                bool isMineralJob = false;
                if (job->line != NULL) {
                    if (job->line->geyser != NULL)
                        isGasJob = true;
                    else
                        isMineralJob = true;
                }
                return valueWorkerAssignmentNoAgent(isGasJob, isMineralJob, job->wishtime);
            }

            if (job == NULL) {
                return valueWorkerAssignmentNoJob(agent->time);
            }

            bool isAssigned = isOldJob(agent, job);
            bool isPlanedWorker = agent->pre != NULL;
            bool isGasJob = false;
            bool isMineralJob = false;
            bool isWorkerIdle = false;
            if (job->line != NULL) {
                if (job->line->geyser != NULL)
                    isGasJob = true;
                else
                    isMineralJob = true;
            }
            if (agent->worker != NULL)
                isWorkerIdle = agent->worker->isIdle();

            return valueWorkerAssignment(agent->time, job->wishtime,
                                         agent->race, job->wishrace,
                                         agent->pos,  job->wishpos,
                                         isPlanedWorker, isGasJob, isMineralJob, isWorkerIdle, isAssigned);
        }

        ctype evaluate(int idAgent, int idJob) const
        {
            if (idAgent >= (int) agents.size()) {
                WorkerJob* job = jobs[idJob];
                return valueCombination(NULL, job);
            } else if (idJob >= (int) jobs.size()) {
                WorkerAgent* agent = agents[idAgent];
                return valueCombination(agent, NULL);
            } else {
                WorkerAgent* agent = agents[idAgent];
                WorkerJob* job = jobs[idJob];
                return valueCombination(agent, job);
            }
        }

        void doAssign(WorkerAgent* agent, WorkerJob* job) const
        {
            bool changed    = !isOldJob(agent, job);

            if (changed && (agent->assigned != NULL))
                agent->assigned->releaseWorker(agent);

            agent->assigned = job;
            if (job != NULL) {
                job->assigned = agent;
                job->update();
            }

            if (changed && (job != NULL))
                job->commandWorker(agent);
        }

        void assign(int idAgent, int idJob) const
        {
            if ((idJob < 0) || (idJob >= (int) jobs.size())) {
                WorkerAgent* agent = agents[idAgent];
                doAssign(agent, NULL);
                return;
            }
            WorkerJob* job = jobs[idJob];

            if ((idAgent < 0) || (idAgent >= (int) agents.size())) {
                job->assigned = NULL;
                return;
            }
            WorkerAgent* agent = agents[idAgent];
            doAssign(agent, job);
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
        if (remove)
            return true;

        if (pre != NULL) {
            if (pre->isFulfilled()) {
                worker = pre->unit;
                time   = 0;
                release(pre);
                if (assigned != NULL)
                    assigned->commandWorker(this);
            } else {
                time = pre->time;
                pos  = pre->pos;
            }
        } else {
            pos = worker->getPosition();
        }

        return false;
    }

    void WorkerAgent::markRemove()
    {
        if (assigned != NULL) {
            assigned->releaseWorker(this);
            assigned->assigned = NULL;
            assigned = NULL;
        }
        remove = true;
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
        wishpos = l->location->getPosition();
    }

    void WorkerJob::markRemoveWithAgent()
    {
        if (assigned != NULL) {
            assigned->markRemove();
            assigned = NULL;
        }
        if (pre != NULL) {
            pre->job = NULL;
            pre      = NULL;
        }
        remove = true;
    }

    void WorkerJob::markRemoveJobOnly()
    {
        if (assigned != NULL) {
            assigned->assigned = NULL;
            assigned = NULL;
        }
        if (pre != NULL) {
            pre->job = NULL;
            pre      = NULL;
        }
        remove = true;
    }

    void WorkerJob::commandWorker(WorkerAgent* agent)
    {
        if (line != NULL) {
            line->commandWorker(agent);
        }
        if (pre != NULL) {
            pre->time = 0;
            pre->unit = agent->worker;
        }
    }

    void WorkerJob::releaseWorker(WorkerAgent* agent)
    {
        if (line != NULL)
            line->releaseWorker(agent);
    }

    bool WorkerJob::update()
    {
        if (remove)
            return true;

        if (pre != NULL) {
            wishtime = pre->wishtime;
            wishpos  = pre->wishpos;
            if (assigned != NULL) {
                pre->time = assigned->time;
                pre->unit = assigned->worker;
                pre->pos  = assigned->pos;
            } else {
                pre->time = Precondition::Impossible;
                pre->unit = NULL;
                pre->pos  = BWAPI::Positions::Unknown;
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
            //LOG << "Commanded worker " << worker << " to gas!";
            target = geyser;
        } else {
            //LOG << "Commanded worker " << worker << " to minerals!";
            target = getNearest(location->getMinerals(), worker->getPosition());
        }
        worker->rightClick(target);

        agents.insert(agent);
    }

    void WorkerLine::releaseWorker(WorkerAgent* agent)
    {
        agents.erase(agent);
    }

    void WorkerLine::addJob()
    {
        jobs.push_back(new WorkerJob(this));
    }

    void WorkerLine::removeJob()
    {
        if (jobs.empty())
            return;
        auto it = jobs.end()-1;
        (*it)->markRemoveJobOnly();
        jobs.erase(it);
    }

    void WorkerLine::setJobCount(int count)
    {
        while (count > (int) jobs.size())
            addJob();
        while (count < (int) jobs.size())
            removeJob();
    }

    void WorkerLine::updateMineralLineJobs()
    {
        updateMineralfields();
        auto count = (3 * mineralfields) / 2;
        if (count < (int) jobs.size())
            THIS_DEBUG << "reducing mineral jobs to " << count << ".";
        setJobCount(count);
    }

    void WorkerLine::onUnitReady()
    {
        if (geyser != NULL) {
            for (int k=0; k<defaultGasWorkerNumber; ++k)
                addJob();
            THIS_DEBUG << "created refinery jobs...";
        } else {
            updateMineralLineJobs();
            THIS_DEBUG << "created base jobs...";
        }
    }

    void WorkerLine::onUnitDestroyed()
    {
        while (!jobs.empty())
            removeJob();
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
        if (geyser != NULL)
            return NULL;

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
    if (!u->getType().isResourceDepot())
        return;

    BWTA::BaseLocation* location = checkBaseLocation(u->getPosition());
    if (location != NULL) {
        THIS_DEBUG << "Base added.";
        new WorkerLine(location, u);
    }
}

UnitPrecondition* registerBase(UnitPrecondition* b)
{
    if (!b->ut.isResourceDepot())
        return b;

    BWTA::BaseLocation* location = checkBaseLocation(b->pos);
    if (location != NULL) {
        THIS_DEBUG << "Planed base added.";
        return (new WorkerLine(location, b))->pre;
    } else {
        THIS_DEBUG << "Registered second base.";
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
        if (pos == NULL)
            continue;

        auto result = buildUnit(pos, type);
        if (result.second != NULL)
            useWorker(result.second);

        if (result.first == NULL)
            continue;

        THIS_DEBUG << "building refinery at " << pos->pos << ".";
        WorkerLine* gasline = new WorkerLine(it->location, result.first, geyser);
        it->gaslines.insert(gasline);
        rememberIdle(gasline->pre);
        return true;
    }
    return false;
}

int freeMineralLinePlaces()
{
    int result = 0;
    for (auto it : lines)
    {
        for (auto jit : it->jobs)
            if (jit->assigned == NULL)
                ++result;
    }
    return result;
}

void setGasWorkerPerRefinery(int nw)
{
    defaultGasWorkerNumber = nw;
    for (auto it : lines)
        for (auto git : it->gaslines)
            if (git->isGasReady())
                git->setJobCount(nw);
}

void MineralLineCode::onMatchBegin()
{
    defaultGasWorkerNumber = 3;
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
    agents.update();
    jobs.update();

    for (auto it : agents)
        if (it->worker != NULL)
            if (!it->worker->getType().isWorker())
    {
        rememberIdle(it->worker);
        it->markRemove();
    }

    Containers::remove_if_delete(lines,  std::mem_fun(&WorkerLine::update));
    Containers::remove_if_delete(agents, std::mem_fun(&WorkerAgent::update));
    Containers::remove_if_delete(jobs,   std::mem_fun(&WorkerJob::update));

    assignment.execute();

    /*
    if (Broodwar->getFrameCount() % 100 == 0)
        if (jobs.size() > 0 || agents.size() > 0)
    {
        LOG << "Solving assignment problem with " << agents.size() << " worker and " << jobs.size() << " jobs.";
        std::stringstream stream;
        stream << "matrix-" << Broodwar->getFrameCount() << ".txt";
        assignment.writeMatrixToFile(stream.str().c_str());
    }
    */
    
    Production& prod = estimatedProduction[0];
    prod.minerals = 1;
    prod.gas      = 1;
    for (auto it : lines) {
        prod.minerals += it->estimateMineralProd();
        prod.gas      += it->estimateGasProd();
    }
}

void MineralLineCode::onCheckMemoryLeak()
{
    WorkerAgent::checkObjectsAlive();
    WorkerJob::checkObjectsAlive();
    WorkerPrecondition::checkObjectsAlive();
    WorkerLine::checkObjectsAlive();
}
