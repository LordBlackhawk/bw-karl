// ToDo:
// *

#include "tech-upgrades.hpp"
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
#include <algorithm>

using namespace BWAPI;

#define THIS_DEBUG DEBUG

namespace
{
    const int savetime = 27;

    struct TechPrecondition;
    std::vector<TechPrecondition*> list;
    bool inOnTick = false;

    struct TechPrecondition : public UnitPrecondition, public ObjectCounter<TechPrecondition>
    {
        enum StatusType { pending, tryagain, commanded, waiting, finished };

        UnitPrecondition*               baseunit;
        ResourcesPrecondition*          resources;
        RequirementsPrecondition*       requirements;
        Precondition*                   extra;
        StatusType                      status;
        Unit*                           worker;
        TechType                        tt;
        UpgradeType                     gt;
        int                             starttime;
        int                             tries;

        TechPrecondition(UnitPrecondition* u, ResourcesPrecondition* r, RequirementsPrecondition* req, const TechType& t, Precondition* e)
            : UnitPrecondition(1, u->ut, u->pos, u->mod),
              baseunit(u), resources(r), requirements(req), extra(e), status(pending),
              worker(NULL), tt(t), gt(UpgradeTypes::None), starttime(0), tries(0)
        {
            updateTime();
        }

        TechPrecondition(UnitPrecondition* u, ResourcesPrecondition* r, RequirementsPrecondition* req, const UpgradeType& t, Precondition* e)
            : UnitPrecondition(1, u->ut, u->pos, u->mod),
              baseunit(u), resources(r), requirements(req), extra(e), status(pending),
              worker(NULL), tt(TechTypes::None), gt(t), starttime(0), tries(0)
        {
            updateTime();
        }

        ~TechPrecondition()
        {
            if (!inOnTick)
                Containers::remove(list, this);

            release(baseunit);
            release(resources);
            release(requirements);
            release(extra);
        }

        int techTime() const
        {
            if (tt != TechTypes::None) {
                return tt.researchTime();
            } else {
                return gt.upgradeTime();
            }
        }

        std::string techName() const
        {
            if (tt != TechTypes::None) {
                return tt.getName();
            } else {
                return gt.getName();
            }
        }

        bool updateTime()
        {
            switch (status)
            {
                case pending:
                    if (updateTimePreconditions(this, ut.buildTime(), baseunit, resources, requirements, extra)) {
                        start();
                        time = Broodwar->getFrameCount() + techTime();
                        THIS_DEBUG << "researching " << techName() << " started.";
                    }
                    break;

                case tryagain:
                    time = Broodwar->getFrameCount() + techTime();
                    if (hasStarted()) {
                        freeResources();
                        status = waiting;
                        THIS_DEBUG << "waiting for research '" << techName() << "' to finish.";
                    } else {
                        start();
                    }
                    break;

                case commanded:
                    time = Broodwar->getFrameCount() + techTime();
                    if (hasStarted()) {
                        freeResources();
                        status = waiting;
                        THIS_DEBUG << "waiting for research '" << techName() << "' to finish.";
                    } else if (Broodwar->getFrameCount() > starttime + savetime) {
                        start();
                        THIS_DEBUG << "research " << techName() << " restarted (try " << tries << ").";
                    }
                    break;

                case waiting:
                    if (isFinished()) {
                        time   = 0;
                        unit   = worker;
                        status = finished;
                        THIS_DEBUG << "research " << techName() << " finished.";
                    }
                    break;

                case finished:
                    break;
            }
            return (status == finished);
        }

        void start()
        {
            if (baseunit != NULL) {
                worker = baseunit->unit;
                release(baseunit);
            }
            assert(worker != NULL);
            if (!action()) {
                auto err = Broodwar->getLastError();
                THIS_DEBUG << "Error: Unable to research '" << techName() << "': " << err.toString();
                if (   (err == Errors::Insufficient_Minerals)
                    || (err == Errors::Insufficient_Gas))
                {
                    status = pending;
                    return;
                }
            }
            status = commanded;
            ++tries;
            starttime = Broodwar->getFrameCount();
        }

        bool action() const
        {
            if (tt != TechTypes::None) {
                return worker->research(tt);
            } else {
                return worker->upgrade(gt);
            }
        }

        bool hasStarted() const
        {
            return !isFinished();
        }

        bool isFinished() const
        {
            if (tt != TechTypes::None) {
                return !worker->isResearching();
            } else {
                return !worker->isUpgrading();
            }
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

UnitPrecondition* researchTech(UnitPrecondition* worker, ResourcesPrecondition* res, const BWAPI::TechType& tt, Precondition* extra)
{
    RequirementsPrecondition* req = getRequirements(tt);
    // req maybe NULL.

    TechPrecondition* result = new TechPrecondition(worker, res, req, tt, extra);
    list.push_back(result);

    return result;
}

UnitPrecondition* researchTech(UnitPrecondition* worker, const BWAPI::TechType& tt, Precondition* extra)
{
    ResourcesPrecondition* res = getResources(tt);
    if (res == NULL)
        return NULL;
    return researchTech(worker, res, tt, extra);
}

UnitPrecondition* researchTech(const BWAPI::TechType& tt, Precondition* extra)
{
    UnitPrecondition* worker = getIdleUnit(tt.whatResearches(), UnitPrecondition::WhatEver);
    if (worker == NULL)
        return NULL;
    return researchTech(worker, tt, extra);
}

UnitPrecondition* upgradeTech(UnitPrecondition* worker, ResourcesPrecondition* res, const BWAPI::UpgradeType& gt, Precondition* extra)
{
    RequirementsPrecondition* req = getRequirements(gt);
    // req maybe NULL.

    TechPrecondition* result = new TechPrecondition(worker, res, req, gt, extra);
    list.push_back(result);

    return result;
}

UnitPrecondition* upgradeTech(UnitPrecondition* worker, const BWAPI::UpgradeType& gt, Precondition* extra)
{
    ResourcesPrecondition* res = getResources(gt);
    if (res == NULL)
        return NULL;
    return upgradeTech(worker, res, gt, extra);
}

UnitPrecondition* upgradeTech(const BWAPI::UpgradeType& gt, Precondition* extra)
{
    UnitPrecondition* worker = getIdleUnit(gt.whatUpgrades(), UnitPrecondition::WhatEver);
    if (worker == NULL)
        return NULL;
    return upgradeTech(worker, gt, extra);
}

void researchTechEx(const BWAPI::TechType& tt)
{
    rememberIdle(researchTech(tt));
}

void upgradeTechEx(const BWAPI::UpgradeType& gt)
{
    rememberIdle(upgradeTech(gt));
}

void TechCode::onMatchEnd()
{
    list.clear();
}

void TechCode::onTick()
{
    inOnTick = true;
    Containers::remove_if(list, std::mem_fun(&TechPrecondition::updateTime));
    inOnTick = false;
}

void TechCode::onDrawPlan(HUDTextOutput& /*hud*/)
{
    //for (auto it : list)
    //    it->onDrawPlan();
}

void TechCode::onCheckMemoryLeaks()
{
    TechPrecondition::checkObjectsAlive();
}
