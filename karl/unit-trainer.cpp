// ToDo:
// *

#include "unit-trainer.hpp"
#include "resources.hpp"
#include "idle-unit-container.hpp"
#include "supply.hpp"
#include "precondition-helper.hpp"
#include "container-helper.hpp"
#include "larvas.hpp"
#include "mineral-line.hpp"
#include "requirements.hpp"
#include "object-counter.hpp"
#include "log.hpp"
#include <map>
#include <vector>
#include <algorithm>
#include <cassert>

using namespace BWAPI;

#define THIS_DEBUG DEBUG

namespace
{
    const int savetime = 27;
    
    struct UnitTrainerPrecondition;
    std::vector<UnitTrainerPrecondition*> list;
    
    UnitPrecondition::ModifierType getAddonModifier(const UnitType& ut)
    {
        if (   (ut == UnitTypes::Terran_Siege_Tank_Tank_Mode)
            || (ut == UnitTypes::Terran_Valkyrie))
        {
            return UnitPrecondition::WithAddon;
        }
        return UnitPrecondition::WhatEver;
    }

    struct UnitTrainerPrecondition : public UnitPrecondition, public ObjectCounter<UnitTrainerPrecondition>
    {
        enum StatusType { pending, tryagain, commanded, waiting, finished };

        UnitPrecondition*               baseunit;
        ResourcesPrecondition*          resources;
        SupplyPrecondition*             supply;
        RequirementsPrecondition*       requirements;
        StatusType                      status;
        UnitPrecondition*               postworker;
        Unit*                           worker;
        int                             starttime;
        int                             tries;

        UnitTrainerPrecondition(UnitPrecondition* u, ResourcesPrecondition* r, SupplyPrecondition* s, RequirementsPrecondition* req, 
                                const UnitType& ut, const std::string& dn)
            : UnitPrecondition(1, ut, Position(u->pos), UnitPrecondition::WithoutAddon, dn),
              baseunit(u), resources(r), supply(s), requirements(req), status(pending), 
              postworker(NULL), worker(NULL), starttime(0), tries(0)
        {
            list.push_back(this);
            updateTime();
            postworker = new UnitPrecondition(Precondition::Impossible, baseunit->ut, baseunit->pos, baseunit->mod);
        }
        
        ~UnitTrainerPrecondition()
        {
            Containers::remove(list, this);

            release(baseunit);
            release(resources);
            release(supply);
            release(requirements);
        }

        bool updateTime()
        {
            switch (status)
            {
                case pending:
                    baseunit->wishpos = wishpos;
                    if (updateTimePreconditions(this, ut.buildTime(), baseunit, resources, supply, requirements)) {
                        start();
                        time = Broodwar->getFrameCount() + ut.buildTime();
                        THIS_DEBUG << "training " << ut << " started.";
                    }
                    break;
                
                case tryagain:
                    time = Broodwar->getFrameCount() + ut.buildTime();
                    if (hasStarted()) {
                        freeResources();
                        status = waiting;
                        THIS_DEBUG << "waiting for trained unit '" << ut << "' to finish.";
                    } else {
                        start();
                    }
                    break;

                case commanded:
                    time = Broodwar->getFrameCount() + ut.buildTime();
                    if (hasStarted()) {
                        freeResources();
                        status = waiting;
                        THIS_DEBUG << "waiting for trained unit '" << ut << "' to finish.";
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
            assert(worker != NULL);
            THIS_DEBUG << "Sending worker to build " << ut;
            if (!worker->train(ut)) {
                auto err = Broodwar->getLastError();
                WARNING << "Unable to train unit '" << ut << "': " << err;
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
                    baseunit = getIdleUnit(ut.whatBuilds().first, getAddonModifier(ut));
                    rememberIdle(worker);
                    worker = NULL;
                    return;
                }
            }
            status = commanded;
            ++tries;
            starttime = Broodwar->getFrameCount();
        }
        
        bool hasStarted() const
        {
            return (unit != NULL);
        }
        
        bool isFinished() const
        {
            return !worker->isTraining();
        }

        void freeResources()
        {
            release(resources);
            release(supply);
            release(requirements);
        }
        
        bool near(const TilePosition& p1, const TilePosition& p2) const
        {
            return (std::abs(p1.x() - p2.x()) <= ut.tileWidth()) && (std::abs(p1.y() - p2.y()) <= ut.tileHeight());
        }
        
        bool onAssignUnit(Unit* u)
        {
            if ((status != commanded) && (status != tryagain))
                return false;
            if (u->getType() != ut)
                return false;
            unit = u;
            return true;
        }
        
        void onDrawPlan(std::map<Position, std::vector<UnitTrainerPrecondition*>>& drawItems)
        {
            if (baseunit != NULL)
                drawItems[baseunit->pos].push_back(this);
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

std::pair<UnitPrecondition*, UnitPrecondition*> trainUnit(UnitPrecondition* worker, ResourcesPrecondition* res, SupplyPrecondition* supply, const BWAPI::UnitType& ut, const std::string& debugname)
{
    // supply maybe NULL.
    if ((worker == NULL) || (res == NULL)) {
        release(worker);
        release(res);
        release(supply);
        return std::pair<UnitPrecondition*, UnitPrecondition*>(NULL, NULL);
    }

    RequirementsPrecondition* req = getRequirements(ut);
    // req maybe NULL.
    
    UnitTrainerPrecondition* result = new UnitTrainerPrecondition(worker, res, supply, req, ut, debugname);
    UnitPrecondition* first  = result;
    UnitPrecondition* second = result->postworker;
    return std::make_pair(first, second);
}

std::pair<UnitPrecondition*, UnitPrecondition*> trainUnit(UnitPrecondition* worker, ResourcesPrecondition* res, const BWAPI::UnitType& ut, const std::string& debugname)
{
    SupplyPrecondition* supply = getSupply(ut);
    return trainUnit(worker, res, supply, ut, debugname);
}

std::pair<UnitPrecondition*, UnitPrecondition*> trainUnit(ResourcesPrecondition* res, const BWAPI::UnitType& ut, const std::string& debugname)
{
    UnitPrecondition* worker = getIdleUnit(ut.whatBuilds().first, getAddonModifier(ut));
    return trainUnit(worker, res, ut, debugname);
}

std::pair<UnitPrecondition*, UnitPrecondition*> trainUnit(const BWAPI::UnitType& ut, const std::string& debugname)
{
    ResourcesPrecondition* res = getResources(ut);
    return trainUnit(res, ut, debugname);
}

void trainUnitEx(const BWAPI::UnitType& ut, const std::string& debugname)
{
    rememberIdle(rememberSecond(trainUnit(ut, debugname)));
}

void trainWorker(const BWAPI::UnitType& ut, const std::string& debugname)
{
    useWorker(rememberSecond(trainUnit(ut, debugname)));
}

void UnitTrainerCode::onMatchEnd()
{
    list.clear();
}

void UnitTrainerCode::onTick()
{
    Containers::remove_if(list, std::mem_fun(&UnitTrainerPrecondition::updateTime));
}

bool UnitTrainerCode::onAssignUnit(BWAPI::Unit* unit)
{
    for (auto it : list)
        if (it->onAssignUnit(unit))
            return true;
    return false;
}

void UnitTrainerCode::onDrawPlan(HUDTextOutput& /*hud*/)
{
    std::map<Position, std::vector<UnitTrainerPrecondition*>> drawItems;
    for (auto it : list)
        it->onDrawPlan(drawItems);
    
    for (auto it : drawItems) {
        Position pos = it.first;
        std::vector<UnitTrainerPrecondition*> queue = it.second;
        std::sort(queue.begin(), queue.end(), PreconditionSorter());
        
        int line = 0;
        for (auto m : queue) {
            Broodwar->drawTextMap(pos.x(), pos.y() + 16 * line, "\x07%d %s", m->time, m->debugname.c_str());
            ++line;
            if (line >= 3)
                break;
        }
        if (queue.size() > 3)
            Broodwar->drawTextMap(pos.x(), pos.y() + 16 * line, "\x07... (%d others)", queue.size()-3);
    }
}

void UnitTrainerCode::onCheckMemoryLeaks()
{
    UnitTrainerPrecondition::checkObjectsAlive();
}
