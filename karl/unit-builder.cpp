// ToDo:
//  * Backup unit, if first is attacked.
//  * Send worker to building location.

#include "unit-builder.hpp"
#include "building-placer.hpp"
#include "resources.hpp"
#include "mineral-line.hpp"
#include "precondition-helper.hpp"
#include "container-helper.hpp"
#include "larvas.hpp"
#include "supply.hpp"
#include "idle-unit-container.hpp"
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

    struct UnitBuilderPrecondition;
    std::vector<UnitBuilderPrecondition*> list;

    struct UnitBuilderPrecondition : public UnitPrecondition, public ObjectCounter<UnitBuilderPrecondition>
    {
        enum StatusType { pending, tryagain, commanded, waiting, finished };

        UnitPrecondition*                 baseunit;
        ResourcesPrecondition*            resources;
        BuildingPositionPrecondition*     pos;
        RequirementsPrecondition*         requirements;
        Precondition*                     extra;
        StatusType                        status;
        UnitPrecondition*                 postworker;
        Unit*                             worker;
        int                               starttime;
        int                               tries;

        UnitBuilderPrecondition(UnitPrecondition* u, ResourcesPrecondition* r, BuildingPositionPrecondition* p, RequirementsPrecondition* req,
                                const UnitType& ut, Precondition* e)
            : UnitPrecondition(1, ut, Position(p->pos)), baseunit(u), resources(r), pos(p), requirements(req), extra(e), status(pending),
              postworker(NULL), worker(NULL), starttime(0), tries(0)
        {
            updateTime();
            if (ut.getRace() == Races::Terran) {
                postworker = new UnitPrecondition(time, baseunit->ut, Position(pos->pos));
            } else if (ut.getRace() == Races::Protoss) {
                postworker = new UnitPrecondition(time - ut.buildTime(), baseunit->ut, Position(pos->pos));
            } else {
                // Zerg worker is consumed.
            }
        }

        ~UnitBuilderPrecondition()
        {
            Containers::remove(list, this);

            release(baseunit);
            release(resources);
            release(pos);
            release(requirements);
            release(extra);
        }

        bool updateTime()
        {
            switch (status)
            {
                case pending:
                    if (updateTimePreconditions(this, ut.buildTime(), baseunit, resources, pos, requirements, extra)) {
                        start();
                        time = Broodwar->getFrameCount() + ut.buildTime();
                        THIS_DEBUG << "building " << ut << " started.";
                    }
                    break;

                case tryagain:
                    time = Broodwar->getFrameCount() + ut.buildTime();
                    if (hasStarted()) {
                        freeResources();
                        status = waiting;
                        THIS_DEBUG << "waiting for building " << ut << " to finish.";
                    } else {
                        start();
                    }
                    break;

                case commanded:
                    time = Broodwar->getFrameCount() + ut.buildTime();
                    if (hasStarted()) {
                        if (ut.getRace() == Races::Protoss)
                            freeWorker();
                        freeResources();
                        status = waiting;
                        THIS_DEBUG << "waiting for building " << ut << " to finish.";
                    } else if (Broodwar->getFrameCount() > starttime + savetime) {
                        start();
                        THIS_DEBUG << "building " << ut << " restarted (try " << tries << ").";
                    }
                    break;

                case waiting:
                    if (isFinished()) {
                        freeWorker();
                        time   = 0;
                        status = finished;
                        THIS_DEBUG << "building " << ut << " finished.";
                    }
                    break;

                case finished:
                    break;
            }
            if (postworker != NULL) {
                int newtime = (ut.getRace() == Races::Protoss) ? time - ut.buildTime() : time;
                postworker->time = std::max(newtime, Broodwar->getFrameCount()+1);
            }
            return (status == finished);
        }

        void start()
        {
            if (baseunit != NULL) {
                worker = baseunit->unit;
                if (ut.getRace() != Races::Zerg) {
                    postworker->unit = worker;
                } else if (ut != UnitTypes::Zerg_Extractor) {
                    unit = worker;
                } else {
                    // Do nothing, worker will not survive.
                }
                release(baseunit);
            }
            if (worker == NULL) {
                WARNING << "UnitBuilder: Got no worker?!?!";
                baseunit = getWorker(ut.getRace());
                status = pending;
                return;
            }
            THIS_DEBUG << "Sending worker to build " << ut;
            if (!pos->isExplored()) {
                worker->rightClick(Position(pos->pos));
                THIS_DEBUG << "Not scouted? Sending Worker...";
            } else if (!worker->build(pos->pos, ut)) {
                auto err = Broodwar->getLastError();
                if (err == Errors::Unit_Busy) {
                    status = tryagain;
                    return;
                }
                WARNING << "Error: Unable to build unit " << ut << ": " << err << "\n"
                        << "\t\tfrom " << worker->getType() << " (player " << worker->getPlayer()->getName() << ")";
                if (err == Errors::Unbuildable_Location) {
                    if (Broodwar->isExplored(pos->pos)) {
                        BuildingPositionPrecondition* newpos = getBuildingPosition(ut);
                        release(pos);
                        status = tryagain;
                        pos    = newpos;
                        return;
                    }
                } else if (err == Errors::Unit_Not_Owned) {
                    status   = pending;
                    unit     = NULL;
                    baseunit = getWorker(ut.getRace());
                    return;
                } else if ((err == Errors::Insufficient_Minerals) || (err == Errors::Insufficient_Gas)) {
                    if (worker != NULL)
                        useWorker(worker);
                    status   = pending;
                    unit     = NULL;
                    baseunit = getWorker(ut.getRace());
                    return;
                }
            }
            status = commanded;
            ++tries;
            starttime = Broodwar->getFrameCount();
        }

        bool hasStarted() const
        {
            if ((ut == UnitTypes::Zerg_Extractor) || (ut.getRace() != Races::Zerg)) {
                return (unit != NULL); //(postworker->getLastCommand().getType() == BWAPI::UnitCommandTypes::Build);
            } else {
                return (unit->getType() == ut);
            }
        }

        bool isFinished() const
        {
            return !unit->isBeingConstructed();
        }

        void freeWorker()
        {
            if (postworker != NULL) {
                postworker->time = 0;
                postworker->unit = worker;
                postworker = NULL;
            }
        }

        void freeResources()
        {
            release(resources);
            release(pos);
            release(requirements);
            release(extra);
        }

        bool near(TilePosition p1, TilePosition p2) const
        {
            return (std::abs(p1.x() - p2.x()) <= ut.tileWidth()) && (std::abs(p1.y() - p2.y()) <= ut.tileHeight());
        }

        bool onAssignUnit(Unit* u)
        {
            if ((status != commanded) && (status != tryagain))
                return false;
            if (u->getType() != ut)
                return false;
            if (!near(u->getTilePosition(), pos->pos))
                return false;
            unit = u;
            return true;
        }

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
            if (status != pending)
                Broodwar->drawTextMap(x+2, y+18, "%s", getStatusText());
            Broodwar->drawTextMap(x+2, y+34, "at %d", time);
            Broodwar->drawTextMap(x+2, y+50, "wish %d", wishtime);
            
            int xvalue = x + 2;
        #define check(PRE, TEXT)                            \
            if (PRE != NULL) if (!PRE->isFulfilled()) {     \
                Broodwar->drawTextMap(xvalue, y+18, (PRE->isImpossible())?"im"#TEXT:"no"#TEXT); \
                xvalue += 30;                               \
            }
            check(baseunit,     Bas);
            check(resources,    Res);
            check(pos,          Pos);
            check(requirements, Req);
            check(extra,        Exr);
        #undef check
        }
    };
}

std::pair<UnitPrecondition*, UnitPrecondition*> buildUnit(UnitPrecondition* worker, ResourcesPrecondition* res, BuildingPositionPrecondition* pos, const BWAPI::UnitType& ut, Precondition* extra)
{
    RequirementsPrecondition* req = getRequirements(ut);
    // req maybe NULL.

    UnitBuilderPrecondition* result = new UnitBuilderPrecondition(worker, res, pos, req, ut, extra);
    list.push_back(result);

    UnitPrecondition* first  = result;
    UnitPrecondition* second = result->postworker;
    if (ut == UnitTypes::Zerg_Hatchery)
        first = registerHatchery(first);
    if (ut.supplyProvided() > 0)
        first = registerSupplyUnit(first);
    if (isRequirement(ut))
        first = registerRequirement(first);
    if (ut.isResourceDepot())
        first = registerBase(first);
    return std::make_pair(first, second);
}

std::pair<UnitPrecondition*, UnitPrecondition*> buildUnit(UnitPrecondition* worker, BuildingPositionPrecondition* pos, const BWAPI::UnitType& ut, Precondition* extra)
{
    ResourcesPrecondition* res = getResources(ut);
    if (res == NULL)
        return std::pair<UnitPrecondition*, UnitPrecondition*>(NULL, NULL);
    return buildUnit(worker, res, pos, ut, extra);
}

std::pair<UnitPrecondition*, UnitPrecondition*> buildUnit(BuildingPositionPrecondition* pos, const BWAPI::UnitType& ut, Precondition* extra)
{
    if (pos == NULL)
        return std::pair<UnitPrecondition*, UnitPrecondition*>(NULL, NULL);
    UnitPrecondition* worker = getWorker(ut.getRace());
    if (worker == NULL)
        return std::pair<UnitPrecondition*, UnitPrecondition*>(NULL, NULL);
    return buildUnit(worker, pos, ut, extra);
}

std::pair<UnitPrecondition*, UnitPrecondition*> buildUnit(const BWAPI::UnitType& ut, Precondition* extra)
{
    BuildingPositionPrecondition* pos = getBuildingPosition(ut);
    if (pos == NULL)
        return std::pair<UnitPrecondition*, UnitPrecondition*>(NULL, NULL);
    return buildUnit(pos, ut, extra);
}

void buildUnitEx(const BWAPI::UnitType& ut)
{
    UnitPrecondition* worker = rememberFirst(buildUnit(ut));
    if (worker != NULL)
        useWorker(worker);
}

int buildUnitPlanSize()
{
    return list.size();
}

int buildUnitPlanSize(const BWAPI::UnitType& ut)
{
    int result = 0;
    for (auto it : list)
        if (it->ut == ut)
            ++result;
    return result;
}

void UnitBuilderCode::onMatchEnd()
{
    list.clear();
}

void UnitBuilderCode::onTick()
{
    Containers::remove_if(list, std::mem_fun(&UnitBuilderPrecondition::updateTime));
}

bool UnitBuilderCode::onAssignUnit(BWAPI::Unit* unit)
{
    for (auto it : list)
        if (it->onAssignUnit(unit))
            return true;
    return false;
}

void UnitBuilderCode::onUnitMorph(BWAPI::Unit* unit)
{
    if ((unit->getType().isRefinery()) && (unit->getPlayer() == Broodwar->self()))
        for (auto it : list)
            if (it->onAssignUnit(unit))
                return;
}

void UnitBuilderCode::onDrawPlan(HUDTextOutput& /*hud*/)
{
    for (auto it : list)
        it->onDrawPlan();
}

void UnitBuilderCode::onCheckMemoryLeaks()
{
    UnitBuilderPrecondition::checkObjectsAlive();
}
