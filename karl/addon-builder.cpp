// ToDo:
// *

#include "addon-builder.hpp"
#include "resources.hpp"
#include "idle-unit-container.hpp"
#include "precondition-helper.hpp"
#include "container-helper.hpp"
#include "larvas.hpp"
#include "mineral-line.hpp"
#include "requirements.hpp"
#include "object-counter.hpp"
#include "log.hpp"
#include <algorithm>
#include <cassert>

using namespace BWAPI;

#define THIS_DEBUG DEBUG

namespace
{
    const int savetime = 27;
    
    struct AddonBuilderPrecondition;
    std::vector<AddonBuilderPrecondition*> list;

    struct AddonBuilderPrecondition : public UnitPrecondition, public ObjectCounter<AddonBuilderPrecondition>
    {
        enum StatusType { pending, tryagain, commanded, waiting, finished };

        UnitPrecondition*               baseunit;
        ResourcesPrecondition*          resources;
        RequirementsPrecondition*       requirements;
        Precondition*                   extra;
        StatusType                      status;
        UnitPrecondition*               postworker;
        Unit*                           worker;
        int                             starttime;
        int                             tries;

        AddonBuilderPrecondition(UnitPrecondition* u, ResourcesPrecondition* r, RequirementsPrecondition* req, 
                                const UnitType& ut, Precondition* e)
            : UnitPrecondition(1, ut, Position(u->pos), UnitPrecondition::WithoutAddon),
              baseunit(u), resources(r), requirements(req), extra(e), status(pending),
              postworker(NULL), worker(NULL), starttime(0), tries(0)
        {
            updateTime();
            postworker = new UnitPrecondition(Precondition::Impossible, baseunit->ut, baseunit->pos, UnitPrecondition::WithAddon);
        }
        
        ~AddonBuilderPrecondition()
        {
            Containers::remove(list, this);

            release(baseunit);
            release(resources);
            release(requirements);
            release(extra);
        }

        bool updateTime()
        {
            switch (status)
            {
                case pending:
                    if (updateTimePreconditions(this, ut.buildTime(), baseunit, resources, requirements, extra)) {
                        start();
                        time = Broodwar->getFrameCount() + ut.buildTime();
                        THIS_DEBUG << "building addon " << ut << " started.";
                    }
                    break;
                
                case tryagain:
                    time = Broodwar->getFrameCount() + ut.buildTime();
                    if (hasStarted()) {
                        freeResources();
                        status = waiting;
                        THIS_DEBUG << "waiting for addon unit '" << ut << "' to finish.";
                    } else {
                        start();
                    }
                    break;

                case commanded:
                    time = Broodwar->getFrameCount() + ut.buildTime();
                    if (hasStarted()) {
                        freeResources();
                        status = waiting;
                        THIS_DEBUG << "waiting for addon unit '" << ut << "' to finish.";
                    } else if (Broodwar->getFrameCount() > starttime + savetime) {
                        start();
                        THIS_DEBUG << "unit " << ut << " restarted (try " << tries << ").";
                    }
                    break;

                case waiting:
                    if (isFinished()) {
                        postworker->time = 0;
                        postworker->unit = worker;
                        postworker = NULL;
                        time   = 0;
                        status = finished;
                        THIS_DEBUG << "unit " << ut << " finished.";
                    }
                    break;
                
                case finished:
                    break;
            }
            if (postworker != NULL)
                postworker->time = time;
            return (status == finished);
        }

        void start()
        {
            if (baseunit != NULL) {
                worker = baseunit->unit;
                release(baseunit);
            }
            if (worker == NULL) {
                WARNING << "AddonBuilder(" << ut << "): Got no worker?!?!";
                baseunit = getIdleUnit(ut.whatBuilds().first, UnitPrecondition::WithoutAddon);
                status   = (baseunit != NULL) ? pending : finished;
                return;
            }
            THIS_DEBUG << "Building addon " << ut;
            if (!worker->buildAddon(ut)) {
                auto err = Broodwar->getLastError();
                WARNING << "Unable to build addon '" << ut << "': " << err;
                if (   (err == Errors::Insufficient_Minerals) 
                    || (err == Errors::Insufficient_Gas)
                    || (err == Errors::Insufficient_Supply))
                {
                    status = pending;
                    return;
                } else if (   (err == Errors::Unit_Not_Owned)
                           || (err == Errors::Insufficient_Tech))
                {
                    status   = pending;
                    baseunit = getIdleUnit(ut.whatBuilds().first, UnitPrecondition::WithoutAddon);
                    rememberIdle(worker);
                    worker = NULL;
                    return;
                }
            }
            status = commanded;
            ++tries;
            starttime = Broodwar->getFrameCount();
        }
        
        bool onAssignUnit(Unit* u)
        {
            if (worker == NULL)
                return false;

            if (worker->getAddon() != u)
                return false;
                
            unit = u;
            return true;
        }
        
        bool hasStarted()
        {
            return (unit != NULL);
        }
        
        bool isFinished() const
        {
            return !worker->isConstructing();
        }

        void freeResources()
        {
            release(resources);
            release(requirements);
            release(extra);
        }
        
        /*
        const char* getStatusText() const
        {
            switch (status)
            {
                case pending:
                    return "pending";
                case tryagain:
                    return "tryagain";
                case commanded:
                    return "commanded";
                case waiting:
                    return "waiting";
                case finished:
                default:
                    return "finished";
            }
        }
        
        void onDrawPlan() const
        {
            int x, y, width = 32*ut.tileWidth(), height = 32*ut.tileHeight();
            if (pos != NULL) {
                Position p(pos->pos);
                x = p.x();
                y = p.y();
            } else {
                Position p = unit->getPosition();
                x = p.x() - width/2;
                y = p.y() - height/2;
            }
            
            Broodwar->drawBoxMap(x, y, x + width, y + height, Colors::Green, false);
            Broodwar->drawTextMap(x+2, y+2,  "%s", ut.getName().c_str());
            Broodwar->drawTextMap(x+2, y+18, "%s", getStatusText());
            Broodwar->drawTextMap(x+2, y+34, "at %d", time);
            Broodwar->drawTextMap(x+2, y+50, "wish %d", wishtime);
        }
        */
    };
}

std::pair<UnitPrecondition*, UnitPrecondition*> buildAddon(UnitPrecondition* worker, ResourcesPrecondition* res, const BWAPI::UnitType& ut, Precondition* extra)
{
    if ((worker == NULL) || (res == NULL)) {
        release(worker);
        release(res);
        release(extra);
        return std::pair<UnitPrecondition*, UnitPrecondition*>(NULL, NULL);
    }

    RequirementsPrecondition* req = getRequirements(ut);
    // req maybe NULL.
    
    AddonBuilderPrecondition* result = new AddonBuilderPrecondition(worker, res, req, ut, extra);
    list.push_back(result);
    
    UnitPrecondition* first  = result;
    UnitPrecondition* second = result->postworker;
    if (isRequirement(ut))
        first = registerRequirement(first);
    return std::make_pair(first, second);
}

std::pair<UnitPrecondition*, UnitPrecondition*> buildAddon(ResourcesPrecondition* res, const BWAPI::UnitType& ut, Precondition* extra)
{
    UnitPrecondition* worker = getIdleUnit(ut.whatBuilds().first, UnitPrecondition::WithoutAddon);
    return buildAddon(worker, res, ut, extra);
}

std::pair<UnitPrecondition*, UnitPrecondition*> buildAddon(const BWAPI::UnitType& ut, Precondition* extra)
{
    ResourcesPrecondition* res = getResources(ut);
    return buildAddon(res, ut, extra);
}

void buildAddonEx(const BWAPI::UnitType& ut)
{
    rememberIdle(rememberSecond(buildAddon(ut)));
}

void AddonBuilderCode::onMatchEnd()
{
    list.clear();
}

void AddonBuilderCode::onTick()
{
    Containers::remove_if(list, std::mem_fun(&AddonBuilderPrecondition::updateTime));
}

bool AddonBuilderCode::onAssignUnit(BWAPI::Unit* unit)
{
    for (auto it : list)
        if (it->onAssignUnit(unit))
            return true;
    return false;
}

void AddonBuilderCode::onDrawPlan(HUDTextOutput& /*hud*/)
{
    //for (auto it : list)
    //    it->onDrawPlan();
}

void AddonBuilderCode::onCheckMemoryLeaks()
{
    AddonBuilderPrecondition::checkObjectsAlive();
}
