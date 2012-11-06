// ToDo:
//  *

#include "larvas.hpp"
#include "precondition-helper.hpp"
#include "object-counter.hpp"
#include "parallel-vector.hpp"
#include "hungarian-algorithm.hpp"
#include "unit-lifetime-observer.hpp"
#include "idle-unit-container.hpp"
#include "container-helper.hpp"
#include "valuing.hpp"
#include "log.hpp"
#include <algorithm>
#include <functional>
#include <set>

using namespace BWAPI;
using namespace std::placeholders;

#define THIS_DEBUG DEBUG

namespace
{
    const int larva_span_time = 300;

    struct LarvaAgent;
    struct LarvaJob;
    struct LarvaPrecondition;
    
    ParallelVector<LarvaAgent*>         agents;
    ParallelVector<LarvaJob*>           jobs;

    struct LarvaAgent : public ObjectCounter<LarvaAgent>
    {
        // Data:
        int          time;
        Position     pos;
        Unit*        larva;
        LarvaJob*    assigned;
        bool         remove;

        LarvaAgent();
        void markRemove();
        bool update();
    };

    struct LarvaJob : public ObjectCounter<LarvaJob>
    {
        // Data:
        int                 wishtime;
        Position            wishpos;
        LarvaPrecondition*  pre;
        LarvaAgent*         assigned;

        LarvaJob(LarvaPrecondition* p);
        void markRemove();
        bool update();
    };

    struct LarvaPrecondition : public UnitPrecondition, public ObjectCounter<LarvaPrecondition>
    {
        LarvaJob*       job;

        LarvaPrecondition()
            : UnitPrecondition(Precondition::Impossible, UnitTypes::Zerg_Larva, Positions::Unknown), job(NULL)
        {
            job = new LarvaJob(this);
        }

        ~LarvaPrecondition()
        {
            job->markRemove();
        }
    };

    LarvaAgent::LarvaAgent()
        : time(Precondition::Impossible), pos(Positions::Unknown), larva(NULL), assigned(NULL), remove(false)
    {
        Containers::add(agents, this);
    }

    void LarvaAgent::markRemove()
    {
        remove = true;
    }

    bool LarvaAgent::update()
    {
        if (remove) {
            delete this;
            return true;
        }

        if (larva != NULL) {
            time = 0;
            pos  = larva->getPosition();
        }

        return false;
    }

    LarvaJob::LarvaJob(LarvaPrecondition* p)
        : wishtime(0), wishpos(Positions::Unknown), pre(p), assigned(NULL)
    {
        Containers::add(jobs, this);
    }

    void LarvaJob::markRemove()
    {
        if (assigned != NULL) {
            assigned->markRemove();
            assigned = NULL;
        }
        pre = NULL;
    }

    bool LarvaJob::update()
    {
        if (pre == NULL) {
            delete this;
            return true;
        }

        wishtime  = pre->wishtime;
        //wishpos   = pre->wishpos;
        if (assigned != NULL) {
            pre->time = assigned->time;
            pre->unit = assigned->larva;
        } else {
            pre->time = Precondition::Impossible;
            pre->unit = NULL;
        }
        return false;
    }

    struct ProblemType
    {
        double evaluate(int idAgent, int idJob) const
        {
            if (idAgent >= agents.size()) {
                return 0.0;
            } else if (idJob >= jobs.size()) {
                LarvaAgent* agent = agents[idAgent];
                return (agent->time == 0) ? 1e10 : 0.0;
            }

            LarvaAgent* agent = agents[idAgent];
            LarvaJob* job = jobs[idJob];
            return valueLarvaAssignment(agent->time, job->wishtime, job->assigned == agent);
        }

        void assign(int idAgent, int idJob) const
        {
            if ((idJob < 0) || (idJob >= jobs.size())) {
                agents[idAgent]->assigned = NULL;
                return;
            }
            LarvaJob* job = jobs[idJob];

            if ((idAgent < 0) || (idAgent >= agents.size())) {
                job->assigned = NULL;
                return;
            }
            LarvaAgent* agent = agents[idAgent];

            agent->assigned = job;
            job->assigned = agent;
            job->update();
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

    struct HatcheryPlaner;
    typedef std::vector<LarvaAgent*>          AgentContainer;
    typedef AgentContainer::iterator          AgentIterator;

    std::vector<HatcheryPlaner*>    hatcheries;
    AgentContainer                  unassigned_agents;
    
    bool isAgentOfHatch(LarvaAgent* agent, Unit* hatch)
    {
        if (agent->larva == NULL)
            return NULL;
        return (agent->larva->getHatchery() == hatch);
    }

    struct HatcheryPlaner : public UnitLifetimeObserver<HatcheryPlaner>, public ObjectCounter<HatcheryPlaner>
    {
        AgentContainer        agents;

        HatcheryPlaner(Unit* u)
            : UnitLifetimeObserver<HatcheryPlaner>(u)
        {
            hatcheries.push_back(this);
        }

        HatcheryPlaner(UnitPrecondition* p)
            : UnitLifetimeObserver<HatcheryPlaner>(p)
        {
            hatcheries.push_back(this);
        }

        void onRemoveFromList()
        {
            Containers::remove(hatcheries, this);
        }
        
        void onUnitReady()
        {
            auto itend = unassigned_agents.end();
            auto it    = std::remove_if(unassigned_agents.begin(), itend, std::bind(isAgentOfHatch, _1, unit));
            agents.insert(agents.end(), it, itend);
            unassigned_agents.erase(it, itend);
        }
        
        void onDrawPlan()
        {
            int assigned = 0;
            int idle     = 0;
            for (auto it : agents) {
                if (it->assigned != NULL)
                    ++assigned;
                if (it->larva != NULL)
                    ++idle;
            }

            UnitType ut;
            Position pos;
            if (unit != NULL) {
                ut  = unit->getType();
                pos = unit->getPosition();
            } else {
                ut  = pre->ut;
                pos = pre->pos;
            }
            int x = pos.x(), y = pos.y(), width = 32*ut.tileWidth(), height = 32*ut.tileHeight();
            if (unit == NULL) {
                x += width/2;
                y += height/2;
            }
            Broodwar->drawTextMap(x-20, y-4, "%i / %i / %i", idle, assigned, agents.size());
        }

        bool distributeLarva(Unit* u);
        void onUnitDestroyed();
        void onUpdateOnline();
        void onUpdateOffline();
        void updateRest(AgentIterator start, int time);
        void addJobs();
    };

    bool hasNoUnit(LarvaAgent* agent)
    {
        return agent->larva == NULL;
    }

    void HatcheryPlaner::onUnitDestroyed()
    {
        Containers::remove_if(agents, hasNoUnit);
        unassigned_agents.insert(unassigned_agents.end(), agents.begin(), agents.end());
        delete this;
    }

    bool checkAgent(LarvaAgent* agent, int* notassigned)
    {
        if (agent->remove)
            return true;

        if (agent->assigned == NULL)
            ++(*notassigned);

        return false;
    }

    void HatcheryPlaner::addJobs()
    {
        int notassigned = 0;
        Containers::remove_if(agents, std::bind(checkAgent, _1, &notassigned));

        if (notassigned == 0) {
            if (agents.size() < 100)
                agents.push_back(new LarvaAgent());
            else
                WARNING << "More than 100 jobs planed per hatch!";
        }
    }

    void HatcheryPlaner::onUpdateOnline()
    {
        addJobs();
        AgentIterator it    = agents.begin();
        AgentIterator itend = agents.end();
        while ((it != itend) && ((*it)->larva != NULL))
            ++it;
        updateRest(it, Broodwar->getFrameCount() + larva_span_time);
    }

    void HatcheryPlaner::onUpdateOffline()
    {
        addJobs();
        updateRest(agents.begin(), pre->time);
    }

    void HatcheryPlaner::updateRest(AgentIterator start, int time)
    {
        AgentIterator end = agents.end();
        while (start != end) {
            (*start)->time  = time;
            (*start)->larva = NULL;
            ++start;
            time += larva_span_time;
        }
    }

    ProblemType                          problem;
    HungarianAlgorithm<ProblemType>      assignment(problem);

    bool HatcheryPlaner::distributeLarva(Unit* u)
    {
        for (auto it : agents)
            if (it->larva == NULL)
        {
            it->larva = u;
            return true;
        }

        THIS_DEBUG << "No agent for larva found, creating new!";
        LarvaAgent* agent = new LarvaAgent();
        agents.push_back(agent);
        agent->larva = u;
        return true;
    }

    bool distributeLarva(Unit* u)
    {
        Unit* hatch = u->getHatchery();
        for (auto planer : hatcheries)
            if (planer->unit == hatch)
        {
            return planer->distributeLarva(u);
        }
        
        THIS_DEBUG << "Hatchery for larva not found, adding to unassigned list.";
        LarvaAgent* agent = new LarvaAgent();
        unassigned_agents.push_back(agent);
        agent->larva = u;
        return true;
    }
}

UnitPrecondition* getLarva()
{
    return new LarvaPrecondition();
}

void registerHatchery(BWAPI::Unit* u)
{
    new HatcheryPlaner(u);
}

UnitPrecondition* registerHatchery(UnitPrecondition* hatch)
{
    return HatcheryPlaner::createObserver(hatch);
}

void LarvaCode::onMatchBegin()
{
    /*
    for (auto it : Broodwar->self()->getUnits())
        if (it->getType() == UnitTypes::Zerg_Hatchery)
    {
        THIS_DEBUG << "Hatchery added.";
        new HatcheryPlaner(it);
    }
    */
}

void LarvaCode::onMatchEnd()
{
    Containers::clear_and_delete(agents);
    Containers::clear_and_delete(jobs);
    Containers::clear_and_delete(hatcheries);
}

void LarvaCode::onTick()
{
    for (auto it : agents)
        if (it->larva != NULL)
            if (it->larva->getType() != UnitTypes::Zerg_Larva)
    {
        rememberIdle(it->larva);
        it->markRemove();
    }
    
    Containers::remove_if(hatcheries, std::mem_fun(&HatcheryPlaner::update));
    
    int notassigned = 0;
    Containers::remove_if(unassigned_agents, std::bind(checkAgent, _1, &notassigned));

    agents.update();
    Containers::remove_if(agents, std::mem_fun(&LarvaAgent::update));

    jobs.update();
    Containers::remove_if(jobs, std::mem_fun(&LarvaJob::update));
    
    assignment.execute();
}

bool LarvaCode::onAssignUnit(BWAPI::Unit* unit)
{
    return (unit->getType() == UnitTypes::Zerg_Larva) ? distributeLarva(unit) : false;
}

void LarvaCode::onDrawPlan()
{
    for (auto it : hatcheries)
        it->onDrawPlan();
}

void LarvaCode::onCheckMemoryLeaks()
{
    LarvaAgent::checkObjectsAlive();
    LarvaJob::checkObjectsAlive();
    LarvaPrecondition::checkObjectsAlive();
    HatcheryPlaner::checkObjectsAlive();
}
